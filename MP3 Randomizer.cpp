/*
* TODO:

*/

#include <map>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <tchar.h>
#include <ShObjIdl.h>
#include <ShlObj.h>
#include <vector>
#include <fstream>
#include <codecvt>
#include <locale>
#include <filesystem>
#include <Commctrl.h>
#include "resource.h"
#include "LS WIN Debug.h"
#include "LS SYSTEM.h"
#include "LS PROTOTYPES.h"
#include "Comparators.h"

#define VERSION _T("v0.5.0")
#define TITLE _T("MP3 Randomizer II")
#define DEFAULT_N  _T("900")
#define ZERO _T("0");
#define MP3 _T(".mp3")
#define WM_COPY_COMPLETE WM_USER + 1
#define GREEN RGB(0,255,0)
#define TITLE_COLOR RGB(255,128,0)

namespace fs = std::filesystem;
constexpr int MAX_INPUT_NUMBER_SIZE = 3 + 1;
constexpr int TOP_M = 5;
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 580;
constexpr int dy = 20;

// Global variables
struct StateInfo {
	std::wstring inputFolder;
	std::wstring outputFolder;
	std::wstring N = DEFAULT_N;
	std::wstring selected = ZERO;
	std::wstring found = ZERO;
	std::vector<std::wstring> fileList;
	std::vector<std::wstring> selectedList;
	std::map<std::wstring, int> mArtistCount;
	std::map<std::wstring, int> mArtistFound;
	std::multimap<int, std::wstring, ComparatorMapKey> topSelected;
	std::multimap<int, std::wstring, ComparatorMapKey> topFound;
};

static TCHAR szWindowClass[] = TITLE;
static TCHAR szTitle[100];
static TCHAR configFile[] = L"MP3 Randomizer.cfg";
static TCHAR listFile[] = L"MP3 Randomizer.list";
static int topY = 5;

HINSTANCE hInst;
HWND hProgressDialog;

// Forward declarations of functions :
void PaintWindow(HWND hWnd, StateInfo* pState);
//void PaintAnalysis(HWND hWnd, StateInfo* pState);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK NumberInputDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK NotYetProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK HelpDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
inline StateInfo* GetAppState(HWND hwnd);
void DebugStateDisplay(StateInfo* pState);
void ReadStateInfoFromFile(StateInfo* pState, const std::wstring& fileName);
void SaveStateInfoToFile(StateInfo* pState, const std::wstring& fileName);
void SaveFileList(StateInfo* pState, const std::wstring& fileName);
void ReadFileList(StateInfo* pState, const std::wstring& fileName);
void ActionFindMusic(HWND hWnd, StateInfo* pState);
void ActionReadFileList(HWND hWnd, StateInfo* pState, const std::wstring& listFileName);
void CreateAnalysis(StateInfo* pState);
void UpdateProgressBar();
DWORD WINAPI CopyFilesThread(LPVOID lpParam);
void CopyFilesWithProgressDialog(HWND hWnd, HWND hProgressDialog, StateInfo* pState);
void ExitApp(HWND hWnd);
std::map<std::wstring, int> AnalyseFileList(std::vector<std::wstring> list);
std::wstring ExtractArtist(std::wstring path);
std::multimap<int, std::wstring, ComparatorMapKey> GetTopMArtists(const std::map<std::wstring, int>& mDictionary, int M);
void HorizontalLine(HDC hdc, int& y);

/*
	MAIN
*/

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {

	StateInfo* pState = new (std::nothrow) StateInfo;
	if (pState == NULL) return 0;
	_stprintf_s(szTitle, _T("%s %s"), TITLE, VERSION);

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex)) {
		MessageBox(NULL, _T("Call to RegisterClassEx failed!"), _T("Error"), NULL);
		return 1;
	}

	hInst = hInstance;

	// Load resources
	HMENU hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU1));

	if (!hMenu) {
		MessageBox(NULL, _T("Failed to load menu resource!"), _T("Error"), MB_ICONERROR);
		return 1;
	}

	// Extract the first (and only) menu from the loaded menu resource
	HMENU hSubMenu = GetSubMenu(hMenu, 0);
	if (!hSubMenu) {
		MessageBox(NULL, _T("Failed to get sub-menu!"), _T("Error"), MB_ICONERROR);
		return 1;
	}

	HWND hWnd = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,									// WS_EX_OVERLAPPEDWINDOW : An optional extended window style.
		szWindowClass,											// szWindowClass: the name of the application
		szTitle,												// szTitle: the text that appears in the title bar
		WS_OVERLAPPEDWINDOW,									// WS_OVERLAPPEDWINDOW: the type of window to create
		CW_USEDEFAULT, CW_USEDEFAULT,							// CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
		WINDOW_WIDTH, WINDOW_HEIGHT,							// initial size (width, length)
		NULL,													// NULL: the parent of this window
		hMenu,													// hMenu: the menu handle loaded from resources
		hInstance,												// hInstance: the first parameter from WinMain
		pState													// pointer to state
	);

	if (!hWnd) {
		MessageBox(NULL, _T("Call to CreateWindow failed!"), _T("Error"), NULL);
		return 1;
	}
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);
	InitCommonControls();
	hProgressDialog = CreateWindowEx(0, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
		10, rcClient.bottom - 40, rcClient.right - rcClient.left - 20, 30,
		hWnd, NULL, hInst, NULL);
	if (hProgressDialog == NULL) {
		MessageBox(NULL, _T("Failed to create progress dialog!"), _T("Error"), MB_ICONERROR);
		return 1;
	}
	ShowWindow(hProgressDialog, SW_HIDE);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// Main message loop:
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	delete pState;
	UnregisterClass(wcex.lpszClassName, hInstance);
	return (int)msg.wParam;
}

/*
	Functions
*/

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	WORD wmId = LOWORD(wParam);
	WORD wmEvent = HIWORD(wParam);
	std::wstring selectedFolderPath;
	INT_PTR callback_result;
	wchar_t gBuffer[MAX_INPUT_NUMBER_SIZE] = { 0 };

	StateInfo* pState;
	if (message == WM_CREATE) {
		CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		pState = reinterpret_cast<StateInfo*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pState);
	}
	else {
		pState = GetAppState(hWnd);
	}

	switch (message) {
	case WM_CREATE:
		ReadStateInfoFromFile(pState, configFile);
		ActionReadFileList(hWnd, pState, listFile);
		break;
	case WM_PAINT:
		PaintWindow(hWnd, pState);
		break;
	case WM_DESTROY:
		if (MessageBox(hWnd, L"Really quit?", TITLE, MB_OKCANCEL) == IDOK) {
			ExitApp(hWnd);
		}
		break;
	case WM_COMMAND:
		switch (wmId) {
		case ID_HELP_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, AboutDlgProc);
			break;
		case ID_SETUP_QUIT:
			if (MessageBox(hWnd, L"Really quit?", TITLE, MB_OKCANCEL) == IDOK) {
				ExitApp(hWnd);
			}
			break;
		case ID_SETUP_INPUTFOLDER:
			selectedFolderPath = ShowFolderBrowserDialog(hWnd);
			pState->inputFolder = selectedFolderPath;
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		case ID_SETUP_OUTPUTFOLDER:
			selectedFolderPath = ShowFolderBrowserDialog(hWnd);
			pState->outputFolder = selectedFolderPath;
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		case ID_SETUP_NUMBEROFFILES:
			wcscpy_s(gBuffer, MAX_INPUT_NUMBER_SIZE, pState->N.c_str());
			callback_result = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG3), hWnd, NumberInputDialogProc, (LPARAM)(&gBuffer));
			if (callback_result == IDOK) {
				pState->N = gBuffer;
			}
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		case ID_ACTION_FINDMUSIC:
			ActionFindMusic(hWnd, pState);
			break;
		case ID_SETUP_SAVECONFIG:
			SaveStateInfoToFile(pState, configFile);
			break;
		case ID_ACTION_SAVELIST:
			SaveFileList(pState, listFile);
			break;
		case ID_ACTION_LOADLIST:
			ActionReadFileList(hWnd, pState, listFile);
			break;
		case ID_ACTION_RANDOMIZE:
			pState->selectedList = SelectRandomElements(pState->fileList, std::stoi(pState->N));
			pState->selected = std::to_wstring(pState->selectedList.size());
			pState->mArtistCount = AnalyseFileList(pState->selectedList);
			pState->topSelected = GetTopMArtists(pState->mArtistCount, TOP_M);
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		case ID_ACTION_COPYTOOUTPUT:
			if (std::stoi(pState->selected) == 0) {
				MessageBox(hWnd, L"No selection created yet.", L"OK", MB_ICONERROR | MB_OK);
				break;
			}
			CopyFilesWithProgressDialog(hWnd, hProgressDialog, pState);
			break;
		case ID_HELP_HELP:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_HELP), hWnd, HelpDialog);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}

void PaintWindow(HWND hWnd, StateInfo* pState) {
	int x1 = 5;
	int x2 = 120;
	int y = topY;
	int dx = 20;
	PAINTSTRUCT ps;
	HDC hdc;
	TCHAR inputFolder[] = _T("Input folder: ");
	TCHAR outputFolder[] = _T("Output folder: ");
	TCHAR selected[] = _T("N Files to copy: ");
	TCHAR selectedNow[] = _T("Files selected: ");
	TCHAR found[] = _T("Files found: ");
	TCHAR cfg[] = _T("Active configuration: ");
	TCHAR topFound[] = _T("Top artist found: ");
	TCHAR topSelected[] = _T("Top artist in selection: ");
	hdc = BeginPaint(hWnd, &ps);

	HPEN hpen = CreatePen(PS_SOLID, 1, GREEN);
	HPEN hpenOld = (HPEN)SelectObject(hdc, hpen);
	SetBkColor(hdc, RGB(0, 0, 0));
	SetTextColor(hdc, TITLE_COLOR);
	FillRect(hdc, &ps.rcPaint, (HBRUSH)GetStockObject(BLACK_BRUSH));
	TextOut(hdc, x1, y, cfg, (int)_tcslen(cfg));
	SetTextColor(hdc, GREEN);
	y += dy;
	x1 += dx;
	x2 += dx;
	TextOut(hdc, x1, y, inputFolder, (int)_tcslen(inputFolder));
	TextOut(hdc, x2, y, pState->inputFolder.c_str(), static_cast<int>(pState->inputFolder.length()));
	y += dy;
	TextOut(hdc, x1, y, outputFolder, (int)_tcslen(outputFolder));
	TextOut(hdc, x2, y, pState->outputFolder.c_str(), static_cast<int>(pState->outputFolder.length()));
	y += dy;
	TextOut(hdc, x1, y, selected, (int)_tcslen(selected));
	TextOut(hdc, x2, y, pState->N.c_str(), static_cast<int>(pState->N.length()));
	y += dy;
	TextOut(hdc, x1, y, found, (int)_tcslen(found));
	TextOut(hdc, x2, y, pState->found.c_str(), static_cast<int>(pState->found.length()));
	y += dy;
	TextOut(hdc, x1, y, selectedNow, (int)_tcslen(selectedNow));
	TextOut(hdc, x2, y, pState->selected.c_str(), static_cast<int>(pState->selected.length()));
	
	x2 = 200;
	HorizontalLine(hdc, y);
	
	if (!pState->topFound.empty()) {
		SetTextColor(hdc, TITLE_COLOR);
		TextOut(hdc, x1-dx, y, topFound, (int)_tcslen(topFound));
		SetTextColor(hdc, GREEN);
		auto it = pState->topFound.begin();
		for (int i = 0; i < TOP_M && it != pState->topFound.end(); ++i, ++it) {
			y += dy;
			TextOut(hdc, x1, y, it->second.c_str(), (int)it->second.length());
			std::wstring intAsString = std::to_wstring(it->first);
			TextOut(hdc, x2, y, intAsString.c_str(), (int)intAsString.length());
		}
		HorizontalLine(hdc, y);
	}
	
	if (!pState->topSelected.empty()) {
		SetTextColor(hdc, TITLE_COLOR);
		TextOut(hdc, x1 - dx, y, topSelected, (int)_tcslen(topSelected));
		SetTextColor(hdc, GREEN);
		auto it = pState->topSelected.begin();
		for (int i = 0; i < TOP_M && it != pState->topSelected.end(); ++i, ++it) {
			y += dy;
			TextOut(hdc, x1, y, it->second.c_str(), (int)it->second.length());
			std::wstring intAsString = std::to_wstring(it->first);
			TextOut(hdc, x2, y, intAsString.c_str(), (int)intAsString.length());
		}
		HorizontalLine(hdc, y);
	}
	
	SelectObject(hdc, hpenOld);
	DeleteObject(hpen);
	EndPaint(hWnd, &ps);
}

void HorizontalLine(HDC hdc, int& y) {
	y += 2 * dy;
	MoveToEx(hdc, 0, y, NULL);
	LineTo(hdc, WINDOW_WIDTH, y);
	y += dy;
}

INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_STATIC2, VERSION);
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK HelpDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	HWND hHelpText = GetDlgItem(hDlg, IDC_HELP_TEXT);
	switch (message) {
	case WM_INITDIALOG:
		if (hHelpText != nullptr) {
			const wchar_t* customText =
				L"File analysis expects file name in format: title - artist.mp3\r\n"
				L"Select input and ouput paths from where you want to copy to.\r\n"
				L"Find your music collection with 'Find music' and store the list for future use.\r\n"
				L"Select how many files you would like to copy.\r\n"
				L"Created randomized list to copy.\r\n"
				L"Copy.\r\n";
			SetWindowText(hHelpText, customText);
		}
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}

	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK NumberInputDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	int number;
	wchar_t* gBuffer;

	switch (message) {
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_NUMBER_EDIT, (wchar_t*)lParam);
		SetWindowLongPtr(hDlg, DWLP_USER, lParam);
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			gBuffer = (wchar_t*)GetWindowLongPtr(hDlg, DWLP_USER);
			GetDlgItemText(hDlg, IDC_NUMBER_EDIT, gBuffer, MAX_INPUT_NUMBER_SIZE);
			number = _ttoi(gBuffer);
			if (number >= 1 && number <= 999) {
				EndDialog(hDlg, IDOK);
				return (INT_PTR)TRUE;
			}
			else {
				SetDlgItemText(hDlg, IDC_ERROR_MSG, _T("Enter a number between 1 and 999"));
			}
			break;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK NotYetProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
	}
	return (INT_PTR)FALSE;
}

inline StateInfo* GetAppState(HWND hwnd) {
	LONG_PTR ptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
	StateInfo* pState = reinterpret_cast<StateInfo*>(ptr);
	return pState;
}

void DebugStateDisplay(StateInfo* pState) {
	OutputDebugString(L"\n******************\n");
	ConsoleLog("Input: ", pState->inputFolder);
	ConsoleLog("Output: ", pState->outputFolder);
	ConsoleLog("N: ", pState->N);
	ConsoleLog("Found: ", pState->found);
	ConsoleLog("Selected: ", pState->selected);
	OutputDebugString(L"******************\n");
	return;
}

void ReadStateInfoFromFile(StateInfo* pState, const std::wstring& fileName) {
	std::wifstream file(fileName);

	if (!file) {
		ConsoleLog("File not found: ", fileName);
		return;
	}

	std::getline(file, pState->inputFolder);
	std::getline(file, pState->outputFolder);
	std::getline(file, pState->N);
	file.close();
	return;
}

void SaveStateInfoToFile(StateInfo* pState, const std::wstring& fileName) {
	std::wofstream file(fileName, std::ios::trunc);

	if (!file) {
		ConsoleLog("Error for file: ", fileName);
		return;
	}

	file << pState->inputFolder << std::endl;
	file << pState->outputFolder << std::endl;
	file << pState->N << std::endl;
	file.close();
	return;
}

void SaveFileList(StateInfo* pState, const std::wstring& fileName) {
	std::ofstream file(fileName, std::ios::trunc);

	if (!file) {
		ConsoleLog("Error for file: ", fileName);
		return;
	}

	for (const std::wstring& element : pState->fileList) {
		int utf8Size = WideCharToMultiByte(CP_UTF8, 0, element.c_str(), -1, nullptr, 0, nullptr, nullptr);
		if (utf8Size > 1) {
			std::string utf8Str(utf8Size - 1, '\0'); // Reserve space for null-terminator
			WideCharToMultiByte(CP_UTF8, 0, element.c_str(), -1, utf8Str.data(), utf8Size, nullptr, nullptr);
			file << utf8Str << std::endl;
		}
	}
	file.close();
}

void ReadFileList(StateInfo* pState, const std::wstring& fileName) {
	std::ifstream file(fileName);

	if (!file) {
		ConsoleLog("Error reading file: ", fileName);
		return;
	}

	std::string line;
	while (std::getline(file, line)) {
		int wideSize = MultiByteToWideChar(CP_UTF8, 0, line.c_str(), -1, nullptr, 0);
		if (wideSize > 1) {
			std::wstring wstr(wideSize - 1, L'\0'); // Reserve space for null-terminator
			MultiByteToWideChar(CP_UTF8, 0, line.c_str(), -1, wstr.data(), wideSize);
			pState->fileList.push_back(wstr);
		}
	}
	file.close();
}

void ActionFindMusic(HWND hWnd, StateInfo* pState) {
	pState->fileList = FindFilesInDirectory(pState->inputFolder, MP3);
	pState->found = std::to_wstring(pState->fileList.size());
	InvalidateRect(hWnd, NULL, TRUE);
	CreateAnalysis(pState);
	return;
}

void ActionReadFileList(HWND hWnd, StateInfo* pState, const std::wstring& listFileName) {
	pState->fileList.clear();
	ReadFileList(pState, listFileName);
	pState->found = std::to_wstring(pState->fileList.size());
	InvalidateRect(hWnd, NULL, TRUE);
	CreateAnalysis(pState);
	return;
}

void CreateAnalysis(StateInfo* pState) {
	pState->mArtistFound = AnalyseFileList(pState->fileList);
	pState->topFound = GetTopMArtists(pState->mArtistFound, TOP_M);
	LogMap(pState->topFound);
	return;
}

void UpdateProgressBar() {
	SendMessage(hProgressDialog, PBM_SETSTEP, 1, 0);
	SendMessage(hProgressDialog, PBM_STEPIT, 0, 0);
	UpdateWindow(hProgressDialog);
}

DWORD WINAPI CopyFilesThread(LPVOID lpParam) {
	auto pState = static_cast<StateInfo*>(lpParam);
	const auto& fileList = pState->selectedList;
	const auto& outputDirectory = pState->outputFolder;
	int numFiles = std::stoi(pState->N);
	SendMessage(hProgressDialog, PBM_SETRANGE32, 0, numFiles);

	for (int i = 0; i < numFiles; ++i) {
		std::wstring sourceFile = fileList[i];
		std::wstring destinationFile = outputDirectory + L"\\" + fs::path(sourceFile).filename().wstring();

		try {
			fs::copy(sourceFile, destinationFile, fs::copy_options::overwrite_existing);
		}
		catch (const fs::filesystem_error& ex) {
			std::wstringstream errorStream;
			errorStream << L"Filesystem error when copying file: " << ex.what();
			ConsoleLog(errorStream.str());
		}
		UpdateProgressBar();
	}

	ShowWindow(hProgressDialog, SW_HIDE);
	PostMessage(hProgressDialog, WM_COPY_COMPLETE, 0, 0);
	return 0;
}

void CopyFilesWithProgressDialog(HWND hWnd, HWND hProgressDialog, StateInfo* pState) {
	DWORD threadId;
	HANDLE hThread = CreateThread(nullptr, 0, CopyFilesThread, pState, 0, &threadId);
	if (hThread == nullptr) {
		DWORD dwError = GetLastError();
		std::wstringstream ws;
		ws << L"Failed to create thread. Error code: " << dwError;
		ConsoleLog(ws.str());
		MessageBox(hWnd, L"Failed to start the file copying operation.", L"Error", MB_ICONERROR | MB_OK);
		return;
	}

	ShowWindow(hProgressDialog, SW_SHOW);
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0)) {
		if (msg.message == WM_COPY_COMPLETE) {
			break;
		}
		if (!IsDialogMessage(hProgressDialog, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
}

void ExitApp(HWND hWnd) {
	PostQuitMessage(0);
	DestroyWindow(hProgressDialog);
	DestroyWindow(hWnd);
}

std::map<std::wstring, int> AnalyseFileList(std::vector<std::wstring> list) {
	std::map<std::wstring, int> mDictionary;
	std::wstring Artist;
	for (const auto& path : list) {
		Artist = ExtractArtist(path);
		if (mDictionary.contains(Artist)) {
			mDictionary[Artist]++;
		}
		else {
			mDictionary.emplace(Artist, 1);
		}
	}
	return mDictionary;
}

std::wstring ExtractArtist(std::wstring path) {
	size_t startPos = path.rfind(L'-') + 2;
	size_t endPos = path.rfind(L'.');
	if (startPos == std::wstring::npos || endPos == std::wstring::npos) return TEXT("Unknown Artist");
	return path.substr(startPos, endPos - startPos);
}

std::multimap<int, std::wstring, ComparatorMapKey> GetTopMArtists(const std::map<std::wstring, int>& mDictionary, int M) {
	std::multimap<int, std::wstring, ComparatorMapKey> sortedMap = SortMapByValueDescending(mDictionary);
	std::multimap<int, std::wstring, ComparatorMapKey> topMMap;
	auto it = sortedMap.begin();
	for (int i = 0; i < M && it != sortedMap.end(); ++i, ++it) {
		topMMap.insert(*it);
	}

	return topMMap;
}
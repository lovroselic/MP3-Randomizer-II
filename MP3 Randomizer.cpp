/*
	https://stackoverflow.com/questions/53642885/how-to-get-text-from-dialogbox-without-global-variable-usage
*/

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <tchar.h>
#include <ShObjIdl.h>
#include <ShlObj.h>
#include "resource.h"

#define VERSION _T("v0.2.1")
#define TITLE _T("MP3 Randomizer II")
#define DEFAULT_N  _T("900")
#define MAX_INPUT_NUMBER_SIZE 3

// Global variables
struct StateInfo {
	std::wstring inputFolder;
	std::wstring outputFolder;
	std::wstring N = DEFAULT_N;
	std::wstring selected = _T("0");
	std::wstring found = _T("0");
};

static TCHAR szWindowClass[] = _T("MP3 Randomizer");
static TCHAR szTitle[100];

HINSTANCE hInst;

// Forward declarations of functions :
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void PaintWindow(HWND hWnd, StateInfo* pState);
INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK NumberInputDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK NotYetProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
std::wstring ShowFolderBrowserDialog(HWND hWnd);
inline StateInfo* GetAppState(HWND hwnd);
void DebugStateDisplay(StateInfo* pState);

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
		MessageBox(NULL, _T("Call to RegisterClassEx failed!"), _T("Windows Desktop Guided Tour"), NULL);
		return 1;
	}

	// Store instance handle in our global variable
	hInst = hInstance;

	// Load the menu resource
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
		800, 400,												// 500, 100: initial size (width, length)
		NULL,													// NULL: the parent of this window
		hMenu,													// hMenu: the menu handle loaded from resources
		hInstance,												// hInstance: the first parameter from WinMain
		pState													// pointer to state
	);

	if (!hWnd) {
		MessageBox(NULL, _T("Call to CreateWindow failed!"), _T("Windows Desktop Guided Tour"), NULL);
		return 1;
	}

	// The parameters to ShowWindow explained:
		// hWnd: the value returned from CreateWindow
		// nCmdShow: the fourth parameter from WinMain
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// Main message loop:
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	delete pState;

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	WORD wmId = LOWORD(wParam);
	WORD wmEvent = HIWORD(wParam);
	std::wstring selectedFolderPath;
	INT_PTR callback_result;
	//int enteredNumber = 0;
	wchar_t gBuffer[MAX_INPUT_NUMBER_SIZE + 1] = { 0 };

	StateInfo* pState;
	if (message == WM_CREATE) {
		CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		pState = reinterpret_cast<StateInfo*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pState);
	}
	else {
		pState = GetAppState(hWnd);
	}

	//std::wstring numberOfFiles = pState->N;

	switch (message) {
	case WM_PAINT:
		PaintWindow(hWnd, pState);
		break;
	case WM_DESTROY:
		if (MessageBox(hWnd, L"Really quit?", TITLE, MB_OKCANCEL) == IDOK) {
			PostQuitMessage(0);
		}
		break;
	case WM_COMMAND:
		switch (wmId) {
		case ID_HELP_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, AboutDlgProc);
			break;
		case ID_SETUP_QUIT:
			if (MessageBox(hWnd, L"Really quit?", TITLE, MB_OKCANCEL) == IDOK) {
				PostQuitMessage(0);
			}
			break;
		case ID_SETUP_INPUTFOLDER:
			selectedFolderPath = ShowFolderBrowserDialog(hWnd);
			pState->inputFolder = selectedFolderPath;
			InvalidateRect(hWnd, NULL, TRUE);
			DebugStateDisplay( pState);
			break;
		case ID_SETUP_OUTPUTFOLDER:
			selectedFolderPath = ShowFolderBrowserDialog(hWnd);
			pState->outputFolder = selectedFolderPath;
			InvalidateRect(hWnd, NULL, TRUE);
			DebugStateDisplay(pState);
			break;
		case ID_SETUP_NUMBEROFFILES:
			
			wcscpy_s(gBuffer, MAX_INPUT_NUMBER_SIZE + 1, pState->N.c_str());
			callback_result = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG3), hWnd, NumberInputDialogProc, (LPARAM)(&gBuffer));
			//callback_result = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG3), hWnd, NumberInputDialogProc, reinterpret_cast<LPARAM>(&numberOfFiles));
			if (callback_result ==IDOK) {
				pState->N = gBuffer;
				OutputDebugString(L"ID_SETUP_NUMBEROFFILES (pState->N): ");
				OutputDebugString(pState->N.c_str());
				OutputDebugString(L"\n");
			}
			InvalidateRect(hWnd, NULL, TRUE);
			DebugStateDisplay(pState);
			break;
		case ID_ACTION_FINDMUSIC:
		case ID_ACTION_SAVELIST:
		case ID_ACTION_LOADLIST:
		case ID_ACTION_RANDOMIZE:
		case ID_ACTION_COPYTOOUTPUT:
		case ID_HELP_HELP:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), hWnd, NotYetProc);
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
	int y = 5;
	int dy = 20;
	PAINTSTRUCT ps;
	HDC hdc;
	TCHAR inputFolder[] = _T("Input folder: ");
	TCHAR outputFolder[] = _T("Output folder: ");
	TCHAR selected[] = _T("N Files to copy: ");
	TCHAR selectedNow[] = _T("Files selected: ");
	TCHAR found[] = _T("Files found: ");
	hdc = BeginPaint(hWnd, &ps);
	FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
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
	EndPaint(hWnd, &ps);
}

// Dialog procedure for the About dialog box
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

INT_PTR CALLBACK NumberInputDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	int number;
	//wchar_t gBuffer[MAX_INPUT_NUMBER_SIZE + 1] = { 0 };
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
			GetDlgItemText(hDlg, IDC_NUMBER_EDIT, gBuffer, MAX_INPUT_NUMBER_SIZE+1);
			number = _ttoi(gBuffer);
			if (number >= 1 && number <= 999) {
				
				OutputDebugString(L"CALLBACK (gBuffer): ");
				OutputDebugString(gBuffer);
				OutputDebugString(L"\n");
		
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
		break;
	}
	return (INT_PTR)FALSE;
}

std::wstring ShowFolderBrowserDialog(HWND hWnd) {
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	std::wstring folderPath;

	if (SUCCEEDED(hr)) {
		IFileDialog* pFileDialog;
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog));

		if (SUCCEEDED(hr)) {
			// Set options for the folder browser dialog
			DWORD dwOptions;
			pFileDialog->GetOptions(&dwOptions);
			pFileDialog->SetOptions(dwOptions | FOS_PICKFOLDERS);

			// Show the folder browser dialog
			if (pFileDialog->Show(hWnd) == S_OK) {
				IShellItem* pItem;
				if (pFileDialog->GetResult(&pItem) == S_OK) {
					PWSTR pszFolderPath;
					if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFolderPath)) && pszFolderPath) {
						// Convert to std::wstring and store in folderPath variable
						folderPath = pszFolderPath;
						CoTaskMemFree(pszFolderPath);
					}
					pItem->Release();
				}
			}
			pFileDialog->Release();
		}
		CoUninitialize();
	}
	return folderPath;
}

inline StateInfo* GetAppState(HWND hwnd) {
	LONG_PTR ptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
	StateInfo* pState = reinterpret_cast<StateInfo*>(ptr);
	return pState;
}

void DebugStateDisplay(StateInfo* pState) {
	OutputDebugString(L"\n******************\n");

	OutputDebugString(L"Input: ");
	OutputDebugString(pState->inputFolder.c_str());
	OutputDebugString(L"\n");

	OutputDebugString(L"Output: ");
	OutputDebugString(pState->outputFolder.c_str());
	OutputDebugString(L"\n");

	OutputDebugString(L"N: ");
	OutputDebugString(pState->N.c_str());
	OutputDebugString(L"\n");

	OutputDebugString(L"Found: ");
	OutputDebugString(pState->found.c_str());
	OutputDebugString(L"\n");

	OutputDebugString(L"Selected: ");
	OutputDebugString(pState->selected.c_str());
	OutputDebugString(L"\n");

	OutputDebugString(L"******************\n");
	return;
}
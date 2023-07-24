/*
	https://learn.microsoft.com/en-us/cpp/windows/walkthrough-creating-windows-desktop-applications-cpp?view=msvc-170
	https://learn.microsoft.com/en-us/windows/win32/learnwin32/your-first-windows-program
	https://learn.microsoft.com/en-us/windows/win32/learnwin32/window-messages
	https://learn.microsoft.com/en-us/windows/win32/learnwin32/module-2--using-com-in-your-windows-program
	https://stackoverflow.com/questions/30135494/win32-api-c-menu-bar
	https://learn.microsoft.com/en-us/windows/win32/winmsg/windows?redirectedfrom=MSDN
	https://learn.microsoft.com/en-us/windows/win32/learnwin32/managing-application-state-

*/

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <tchar.h>
#include <ShObjIdl.h>
#include <ShlObj.h>

#include "resource.h"

#define VERSION _T("v0.1.4")
#define TITLE _T("MP3 Randomizer II")

// Global variables
struct StateInfo {
	std::wstring inputFolder;
	std::wstring outputFolder;
};

// The main window class name.
static TCHAR szWindowClass[] = _T("MP3 Randomizer");
static TCHAR szTitle[100];

// Stored instance handle for use in Win32 API calls such as FindResource
HINSTANCE hInst;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void PaintWindow(HWND hWnd);
INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK NotYetProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
std::wstring ShowFolderBrowserDialog(HWND hWnd);
inline StateInfo* GetAppState(HWND hwnd);
void DebugStateDisplay(HWND hWnd);

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {

	StateInfo* pState = new (std::nothrow) StateInfo;

	if (pState == NULL) {
		return 0;
	}

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
		500, 200,												// 500, 100: initial size (width, length)
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

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	WORD wmId = LOWORD(wParam);
	WORD wmEvent = HIWORD(wParam);
	std::wstring selectedFolderPath;

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
	case WM_PAINT:
		PaintWindow(hWnd);
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
			DebugStateDisplay(hWnd);
			break;
		case ID_SETUP_OUTPUTFOLDER:
			selectedFolderPath = ShowFolderBrowserDialog(hWnd);
			pState->outputFolder = selectedFolderPath;
			DebugStateDisplay(hWnd);
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
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}
void PaintWindow(HWND hWnd) {
	PAINTSTRUCT ps;
	HDC hdc;
	TCHAR greeting[] = _T("Hello, Windows desktop!");
	hdc = BeginPaint(hWnd, &ps);
	FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
	TextOut(hdc, 5, 5, greeting, (int)_tcslen(greeting));
	EndPaint(hWnd, &ps);
}

// Dialog procedure for the About dialog box
INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_INITDIALOG:
		// Initialize dialog box controls and data here
		SetDlgItemText(hDlg, IDC_STATIC2, VERSION);
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK NotYetProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_INITDIALOG:
		// Initialize dialog box controls and data here
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

void DebugStateDisplay(HWND hWnd) {
	StateInfo* pState;
	pState = GetAppState(hWnd);

	OutputDebugString(L"Input: ");
	OutputDebugString(pState->inputFolder.c_str());
	OutputDebugString(L"\n");

	OutputDebugString(L"Output: ");
	OutputDebugString(pState->outputFolder.c_str());
	OutputDebugString(L"\n");

	OutputDebugString(L"\n");
	return;
}
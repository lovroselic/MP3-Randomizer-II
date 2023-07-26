/* version 0 */

#include <ShObjIdl.h>
#include <ShlObj.h>
#include <string>
#include <Windows.h>
#include <vector>

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

void FindFilesWithExtension(const std::wstring& directory, const std::wstring& extension, std::vector<std::wstring>& fileList) {
	std::wstring searchPath = directory + L"\\*.*";
	WIN32_FIND_DATA findData;
	HANDLE hFind = FindFirstFile(searchPath.c_str(), &findData);

	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				// Skip "." and ".." directories
				if (wcscmp(findData.cFileName, L".") != 0 && wcscmp(findData.cFileName, L"..") != 0) {
					std::wstring subdir = directory + L"\\" + findData.cFileName;
					FindFilesWithExtension(subdir, extension, fileList);
				}
			}
			else {
				// Check if the file has the specified extension
				std::wstring fileName = findData.cFileName;
				if (fileName.length() >= extension.length() &&
					fileName.compare(fileName.length() - extension.length(), extension.length(), extension) == 0) {
					fileList.push_back(directory + L"\\" + fileName);
				}
			}
		} while (FindNextFile(hFind, &findData) != 0);

		FindClose(hFind);
	}
}

std::vector<std::wstring> FindFilesInDirectory(const std::wstring& path, const std::wstring& extension) {
	std::vector<std::wstring> fileList;
	FindFilesWithExtension(path, extension, fileList);
	return fileList;
}
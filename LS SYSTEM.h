#pragma once
#pragma once
#ifndef LS_SYSTEM
#define LS_SYSTEM
#include <Windows.h>
#include <vector>
#include <string>

std::wstring ShowFolderBrowserDialog(HWND hWnd);
void FindFilesWithExtension(const std::wstring& directory, const std::wstring& extension, std::vector<std::wstring>& fileList);
std::vector<std::wstring> FindFilesInDirectory(const std::wstring& path, const std::wstring& extension);

#endif // !LS_SYSTEM
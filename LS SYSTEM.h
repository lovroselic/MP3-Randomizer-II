#pragma once
#ifndef LS_SYSTEM
#define LS_SYSTEM
#include <Windows.h>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

// forward declarations
std::wstring ShowFolderBrowserDialog(HWND hWnd);
void FindFilesWithExtension(const std::wstring& directory, const std::wstring& extension, std::vector<std::wstring>& fileList);
std::vector<std::wstring> FindFilesInDirectory(const std::wstring& path, const std::wstring& extension);
double getTotalPathSize(const std::vector<std::wstring>& paths);

//templates
template <typename T>
std::wstring fpToWstring(T value, int precision) {
    static_assert(std::is_floating_point<T>::value, "Template argument must be a floating-point type");
    std::wstringstream wss;
    wss << std::fixed << std::setprecision(precision) << value;
    return wss.str();
}

#endif // !LS_SYSTEM
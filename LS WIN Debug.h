#pragma once
#ifndef LS_WIN_DEBUG
#define LS_WIN_DEBUG

#include <sstream>

template <typename PropertyT>
void ConsoleLog(const char* label, const PropertyT& property) {
    std::basic_stringstream<typename PropertyT::value_type> wss;
    wss << label;
    OutputDebugString(wss.str().c_str());
    wss.str(L""); 
    wss << property;
    OutputDebugString(wss.str().c_str());
    OutputDebugString(L"\n");
}

template<typename T>
std::wstring to_wstring_custom(const T& value) {
    return std::to_wstring(value);
}

template<>
std::wstring to_wstring_custom<std::wstring>(const std::wstring& value) {
    return value;
}

template<typename T>
void LogVector(const std::vector<T>& vec) {
    for (const T& element : vec) {
        std::wstring str = to_wstring_custom(element);
        OutputDebugString(str.c_str());
        OutputDebugString(L"\n");
    }
}
#endif // !LS_WIN_DEBUG

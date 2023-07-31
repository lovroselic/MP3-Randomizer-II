#pragma once
#ifndef LS_WIN_DEBUG
#define LS_WIN_DEBUG

#include <sstream>


//forward declarations
void ConsoleLog(const char* message);

//templates

template <typename T>
void ConsoleLog(const T& message ) {
    std::basic_stringstream<typename T::value_type> wss;
    wss << message;
    OutputDebugString(wss.str().c_str());
    OutputDebugString(L"\n");
}

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

template <typename MapType>
void LogMap(const MapType& map) {
    for (const auto& el : map) {
        std::wstring str1 = to_wstring_custom(el.first);
        OutputDebugString(str1.c_str());
        OutputDebugString(L":\t");
        std::wstring str2 = to_wstring_custom(el.second);
        OutputDebugString(str2.c_str());
        OutputDebugString(L"\n");
    }
}


#endif // !LS_WIN_DEBUG

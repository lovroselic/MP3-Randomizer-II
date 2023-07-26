#pragma once
#ifndef LS_WIN_DEBUG
#define LS_WIN_DEBUG

#include <windows.h>
#include <string>
#include <sstream>

template <typename PropertyT>
void ConsoleLog(const char* label, const PropertyT& property) {
    std::basic_stringstream<typename PropertyT::value_type> wss;
    wss << label;
    OutputDebugString(wss.str().c_str());

    wss.str(L""); // Clear the stringstream for the next use.

    wss << property;
    OutputDebugString(wss.str().c_str());
    OutputDebugString(L"\n");
}


#endif // !LS_WIN_DEBUG

/* version 1.0 */

#include <Windows.h>
#include <sstream>

void ConsoleLog(const char* message) {
    std::wstring w_message(message, message + strlen(message));
    OutputDebugString(w_message.c_str());
    OutputDebugString(L"\n");
}
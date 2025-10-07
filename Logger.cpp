#include "Logger.h"
#include <windows.h>

namespace DXE {
    Logger* Logger::s_Logger = nullptr;
    void Logger::Init(Logger* logger, const char* src) {
        if (!logger) {
            s_Logger = new Logger();
            DXE_WARN("Logger Created: " + s_Logger->name + " : ", s_Logger);
        }
        else {
            s_Logger = logger;
            if (src) { s_Logger->source = src; }
            DXE_WARN("Logger Set: " + s_Logger->name + " : ", s_Logger);
        }
    }

	void Logger::AllocateConsole() {
        if (AllocConsole()) {

            // Redirect stdout, stderr, and stdin to the console
            FILE* f;
            if (freopen_s(&f, "CONOUT$", "w", stdout) != 0) {
                std::cerr << "Failed to redirect stdout." << std::endl;}
            if (freopen_s(&f, "CONOUT$", "w", stderr) != 0) {
                std::cerr << "Failed to redirect stderr." << std::endl;}
            if (freopen_s(&f, "CONIN$", "r", stdin) != 0) {
                std::cerr << "Failed to redirect stdin." << std::endl;}

            // Set console title
            //SetConsoleTitle(L"MyApp Console");

            // Get the console handle for the output
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

            // Check if it's valid
            if (hConsole == INVALID_HANDLE_VALUE)
                return;

            // Get the current console mode
            DWORD dwMode = 0;
            if (!GetConsoleMode(hConsole, &dwMode))
                return;

            // Enable ANSI escape sequences
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hConsole, dwMode);
            // Output a test message to ensure it appears
            //DXE_LOG("Console enabled.");
            DXE_INFO("Console enabled.");
            //DXE_WARN("Console enabled.");
            //DXE_ERROR("Console enabled.");
        }
        else {
            DWORD error = GetLastError();
            std::cerr << "Failed to allocate console. Error code: " << error << std::endl;
        }
	}
}
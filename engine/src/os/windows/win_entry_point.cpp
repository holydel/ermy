#include "../os_entry_point.h"
#include <iostream>
#include "win_utils.h"
#include "../../logger.h"
#include <thread>
#include "../os_utils.h"
#include "application.h"
#include <timeapi.h>

#pragma comment(lib,"Winmm.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Ole32.lib")

void PlatformRun(int argc, char* argv[], HINSTANCE hInstance, int nCmdShow)
{
    timeBeginPeriod(1);
    SetProcessDPIAware();

    ErmyApplicationRun();

    timeEndPeriod(1);
}

int main(int argc, char* argv[])
{
    std::cout << "engine entry point main()" << std::endl;
    HINSTANCE myModule = GetModuleHandle(nullptr);
    PlatformRun(argc, argv, GetModuleHandle(nullptr), SW_SHOW);
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR lpCmdLine, int nCmdShow)
{
    std::cout << "engine entry point WinMain" << std::endl;

    int argc = 1;
    char* argv[1] = { lpCmdLine };

    PlatformRun(argc, argv, GetModuleHandle(nullptr), SW_SHOW);

    return 0;
}
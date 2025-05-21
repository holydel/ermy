#include "../os_entry_point.h"
#include <iostream>
#include "win_utils.h"
#include "../../logger.h"
#include <thread>
#include "../os_utils.h"
#include "application.h"
#include <timeapi.h>
#include "win_utils.h"
#include <atomic>
#include <ermy_application.h>
#include "win_gamepads.h"

#pragma comment(lib, "xinput.lib")
#pragma comment(lib,"Winmm.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Ole32.lib")

std::thread* gXInputCheckThread = nullptr;
std::atomic<bool> gXInputCheckThreadRunning(true);
std::array<std::atomic<bool>, ermy::input::gamepad::MAX_CONTROLLERS> gXInputConnected = { false, false, false, false };
std::array<ermy::u8, ermy::input::gamepad::MAX_CONTROLLERS> gControllerBatteryLevel = { 0, 0, 0, 0 };

void GetXInputBatteryState(int index)
{
    XINPUT_BATTERY_INFORMATION batteryInfo;
    DWORD result = XInputGetBatteryInformation(index, XINPUT_DEVTYPE_GAMEPAD, &batteryInfo);

    if (SUCCEEDED(result))
    {
        if (batteryInfo.BatteryLevel == BATTERY_LEVEL_EMPTY)
            gControllerBatteryLevel[index] = 0;
        else if (batteryInfo.BatteryLevel == BATTERY_LEVEL_LOW)
            gControllerBatteryLevel[index] = 64;
        else if (batteryInfo.BatteryLevel == BATTERY_LEVEL_MEDIUM)
            gControllerBatteryLevel[index] = 128;
        else if (batteryInfo.BatteryLevel == BATTERY_LEVEL_FULL)
            gControllerBatteryLevel[index] = 255;
    }
}

void XInputCheckThread()
{
	int controllerToCheck = 0;
    const auto& inputConfig = GetApplication().staticConfig.inputConfig;

    while (gXInputCheckThreadRunning)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(inputConfig.checkGamepadIntervalMS));

        if (!gXInputConnected[controllerToCheck])
        {
            XINPUT_STATE state;
            ZeroMemory(&state, sizeof(XINPUT_STATE));

            if (XInputGetState(controllerToCheck, &state) == ERROR_SUCCESS)
            {
                gXInputConnected[controllerToCheck] = true;
                GetXInputBatteryState(controllerToCheck);
            }
        }
        else
        {
            GetXInputBatteryState(controllerToCheck);
        }

        controllerToCheck = (controllerToCheck + 1) % ermy::input::gamepad::MAX_CONTROLLERS;
    }
}

void PlatformRun(int argc, char* argv[], HINSTANCE hInstance, int nCmdShow)
{
    timeBeginPeriod(1);
    SetProcessDPIAware();

	SetConsoleOutputCP(CP_UTF8);

	gXInputCheckThread = new std::thread(XInputCheckThread);

    ErmyApplicationRun();

	gXInputCheckThreadRunning = false;
	gXInputCheckThread->join();
	delete gXInputCheckThread;
	gXInputCheckThread = nullptr;

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
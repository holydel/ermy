#pragma once
#include <Xinput.h>
#include <array>
#include <atomic>
#include <ermy_input.h>

extern std::array<std::atomic<bool>, ermy::input::gamepad::MAX_CONTROLLERS> gXInputConnected;
extern std::array<ermy::u8, ermy::input::gamepad::MAX_CONTROLLERS> gControllerBatteryLevel;

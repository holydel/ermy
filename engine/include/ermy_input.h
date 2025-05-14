#pragma once
#include <ermy_api.h>
#include <glm/glm.hpp>

namespace ermy
{
	namespace input
	{
		namespace mouse
		{
			glm::ivec2 GetCurrentPosition();
			glm::vec2 GetDeltaPosition();
		}

		namespace keyboard
		{
			enum class KeyCode
			{
				Unknown = 0,
				A = 1,
				B = 2,
				C = 3,
				D = 4,
				E = 5,
				F = 6,
				G = 7,
				H = 8,
				I = 9,
				J = 10,
				K = 11,
				L = 12,
				M = 13,
				N = 14,
				O = 15,
				P = 16,
				Q = 17,
				R = 18,
				S = 19,
				T = 20,
				U = 21,
				V = 22,
				W = 23,
				X = 24,
				Y = 25,
				Z = 26,
				Num0 = 27,
				Num1 = 28,
				Num2 = 29,
				Num3 = 30,
				Num4 = 31,
				Num5 = 32,
				Num6 = 33,
				Num7 = 34,
				Num8 = 35,
				Num9 = 36,
				Space = 37,
				Enter = 38,
				Backspace = 39,
				Tab = 40,
				LeftShift = 41,
				LeftCtrl = 42,
				LeftAlt = 43,
				LeftMeta = 44,
				RightShift = 45,
				RightCtrl = 46,
				RightAlt = 47,
				RightMeta = 48,
				Left = 49,
				Right = 50,
				Up = 51,
				Down = 52,
				Insert = 53,
				Delete = 54,
				Home = 55,
				End = 56,
				PageUp = 57,
				PageDown = 58,
				ScrollLock = 59,
				PrintScreen = 60,
				Pause = 61,
				F1 = 62,
				F2 = 63,
				F3 = 64,
				F4 = 65,
				F5 = 66,
				F6 = 67,
				F7 = 68,
				F8 = 69,
				F9 = 70,
				F10 = 71,
				F11 = 72,
				F12 = 73,
				F13 = 74,
				F14 = 75,
				F15 = 76,
				Escape = 77,
			};

			bool IsKeyPressed(KeyCode key);
			bool IsKeyReleased(KeyCode key);
			bool IsKeyDown(KeyCode key);
		}

		namespace gamepad
		{
			struct GamePadState
			{
				glm::vec2 leftStick;
				glm::vec2 rightStick;
				float leftTrigger;
				float rightTrigger;
				bool A, B, X, Y, LB, RB, Back, Start, LS, RS, Up, Down, Left, Right;
			};

			bool IsConnected(int controllerIndex);
			ermy::u8 GetControllerBatteryLevel(int controllerIndex);
			const GamePadState& GetState(int controllerIndex);

			constexpr int MAX_CONTROLLERS = 4;
		}

		namespace joystick
		{
		}

		namespace xr
		{
			//left controller, right controller, hands, gestures, fullbody, eyegaze
		}
	}
}

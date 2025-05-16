#pragma once
#include "win_utils.h"
#include "application.h"
#include <windowsx.h>
#include "win_gamepads.h"

using namespace ermy;

HWND gMainWindow = nullptr;
HINSTANCE gWinSystemInstance = nullptr;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

constexpr const wchar_t* winClassName = L"ErmyWindow";

std::array<ermy::input::gamepad::GamePadState, ermy::input::gamepad::MAX_CONTROLLERS> gGamePadStates = {};

void RegisterRawInputDevices(bool isFullScreen)
{
	RAWINPUTDEVICE rid[2] = {};

	constexpr USHORT UsagePageGenericDesktop = 0x01;
	constexpr USHORT UsagePageSimulationControls = 0x02;
	constexpr USHORT UsagePageVRControls = 0x03;
	constexpr USHORT UsagePageSportControls = 0x04;
	constexpr USHORT UsagePageGameControls = 0x05;

	constexpr USHORT UsageMouse = 0x02;
	constexpr USHORT UsageJoystick = 0x04;
	constexpr USHORT UsageGamepad = 0x05;
	constexpr USHORT UsageKeyboard = 0x06;
	constexpr USHORT UsageTablet = 0x08;
	constexpr USHORT UsageTouch = 0x01;
	constexpr USHORT UsageTouchPad = 0x0D;
	//RIDEV_NOLEGACY
	rid[0].usUsagePage = UsagePageGenericDesktop;
	rid[0].usUsage = UsageMouse;
	rid[0].dwFlags = RIDEV_INPUTSINK;
	rid[0].hwndTarget = gMainWindow;

	rid[1].usUsagePage = UsagePageGenericDesktop;
	rid[1].usUsage = UsageKeyboard;
	rid[1].dwFlags = RIDEV_INPUTSINK;
	rid[1].hwndTarget = gMainWindow;

	if(isFullScreen)
	{
		//rid[0].dwFlags |= RIDEV_NOLEGACY;
		//rid[1].dwFlags |= RIDEV_NOLEGACY;
	}

	if (!RegisterRawInputDevices(rid, std::size(rid), sizeof(rid[0]))) {
		ERMY_ERROR("Failed to register raw input devices: %d", GetLastError());
	}
}

void* os::CreateNativeWindow()
{
	gWinSystemInstance = GetModuleHandleA(nullptr);
	auto& config = GetApplication().staticConfig;
	auto& winCfg = GetApplication().staticConfig.window;

	const char* utf8WinCaption = config.GetWindowTitle().c_str();
	bool is_maximized = winCfg.initialState == Application::StaticConfig::WindowConfig::InitialState::Maximized;

	WNDCLASSEXW windowClass = {};

	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = &WndProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = gWinSystemInstance;
	windowClass.hIcon = ::LoadIcon(gWinSystemInstance, IDI_APPLICATION);
	windowClass.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	windowClass.lpszMenuName = nullptr;
	windowClass.lpszClassName = winClassName;
	windowClass.hIconSm = ::LoadIcon(gWinSystemInstance, IDI_APPLICATION);

	static ATOM atom = ::RegisterClassExW(&windowClass);

	int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

	RECT windowRect = { 0, 0, static_cast<LONG>(winCfg.width), static_cast<LONG>(winCfg.height) };
	::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	int windowWidth = windowRect.right - windowRect.left;
	int windowHeight = windowRect.bottom - windowRect.top;

	// Center the window within the screen. Clamp to 0, 0 for the top-left corner.
	int windowX = std::max<int>(0, (screenWidth - windowWidth) / 2);
	int windowY = std::max<int>(0, (screenHeight - windowHeight) / 2);

	wchar_t winTitleBuff[256] = { 0 };
	UTF8ToWCS(utf8WinCaption, winTitleBuff);

	DWORD exFlags = 0;
	DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

	if (winCfg.supportTransparent)
	{
		exFlags |= (WS_EX_LAYERED | WS_EX_TRANSPARENT);
	}

	if (winCfg.isFullScreen())
	{
		//exFlags |= WS_EX_TOPMOST;
		dwStyle = WS_VISIBLE | WS_POPUP;

		windowX = 0;
		windowY = 0;
		windowWidth = screenWidth - 0;
		windowHeight = screenHeight - 0;
	}

	HWND hWnd = ::CreateWindowExW(
		exFlags,
		winClassName,
		winTitleBuff,
		dwStyle,
		windowX,
		windowY,
		windowWidth,
		windowHeight,
		nullptr,
		nullptr,
		gWinSystemInstance,
		nullptr
	);

	HMONITOR hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFOEX monitorInfo = {};
	monitorInfo.cbSize = sizeof(MONITORINFOEX);
	::GetMonitorInfo(hMonitor, &monitorInfo);

	if (is_maximized)
	{
		::ShowWindow(hWnd, SW_SHOWMAXIMIZED);
	}
	else
	{
		::ShowWindow(hWnd, SW_SHOW);
	}

	::UpdateWindow(hWnd);


	gMainWindow = hWnd;

	RegisterRawInputDevices(winCfg.isFullScreen());

	return static_cast<void*>(gMainWindow);
}

LONG mouseRawDeltaX = 0;
LONG mouseRawDeltaY = 0;

void ProcessKeyboardAndMouseRAW()
{
	mouseRawDeltaX = 0;
	mouseRawDeltaY = 0;

	// Process raw input using GetRawInputBuffer keyboad and mouse
	BYTE buffer[1024];
	UINT cbSize = sizeof(buffer);
	while (true) {
		UINT nInput = GetRawInputBuffer((PRAWINPUT)buffer, &cbSize, sizeof(RAWINPUTHEADER));
		if (nInput == 0) break;  // No more input
		if (nInput == (UINT)-1) {
			// Handle error (for simplicity, just break)
			break;
		}

		PRAWINPUT pRawInput = (PRAWINPUT)buffer;
		for (UINT i = 0; i < nInput; i++) {
			if (pRawInput->header.dwType == RIM_TYPEMOUSE) {
				mouseRawDeltaX += pRawInput->data.mouse.lLastX;
				mouseRawDeltaY += pRawInput->data.mouse.lLastY;

				//ERMY_LOG("Mouse raw X: %d Y: %d\n", mouseRawDeltaX, mouseRawDeltaY);
				// Add mouse movement handling here (e.g., update camera or cursor)
			}
			else if (pRawInput->header.dwType == RIM_TYPEKEYBOARD) {
				USHORT keyCode = pRawInput->data.keyboard.VKey;
				USHORT flags = pRawInput->data.keyboard.Flags;
				// Add keyboard press/release handling here (e.g., check flags for key state)
			}
			// Move to the next RAWINPUT structure
			pRawInput = (PRAWINPUT)((PBYTE)pRawInput + pRawInput->header.dwSize);
		}
	}
}

void ProcessGamepadsXINPUT()
{
	for(int i=0;i<input::gamepad::MAX_CONTROLLERS;i++)
	{
		if (gXInputConnected[i])
		{
			XINPUT_STATE state;
			ZeroMemory(&state, sizeof(XINPUT_STATE));

			if (XInputGetState(i, &state) == ERROR_SUCCESS)
			{
				auto& s = gGamePadStates[i];

				if(state.Gamepad.sThumbLX >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
					s.leftStick.x = float(state.Gamepad.sThumbLX - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) / float(32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
				else if (state.Gamepad.sThumbLX <= -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
					s.leftStick.x = float(state.Gamepad.sThumbLX + XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) / float(32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
				else
					s.leftStick.x = 0.0f;

				if (state.Gamepad.sThumbLY >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
					s.leftStick.y = float(state.Gamepad.sThumbLY - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) / float(32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
				else if (state.Gamepad.sThumbLY <= -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
					s.leftStick.y = float(state.Gamepad.sThumbLY + XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) / float(32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
				else
					s.leftStick.y = 0.0f;

				if (state.Gamepad.sThumbRX >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
					s.rightStick.x = float(state.Gamepad.sThumbRX - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) / float(32767 - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
				else if (state.Gamepad.sThumbRX <= -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
					s.rightStick.x = float(state.Gamepad.sThumbRX + XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) / float(32767 - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
				else
					s.rightStick.x = 0.0f;

				if (state.Gamepad.sThumbRY >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
					s.rightStick.y = float(state.Gamepad.sThumbRY - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) / float(32767 - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
				else if (state.Gamepad.sThumbRY <= -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
					s.rightStick.y = float(state.Gamepad.sThumbRY + XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) / float(32767 - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
				else
					s.rightStick.y = 0.0f;

				// Triggers
				s.leftTrigger = state.Gamepad.bLeftTrigger / 255.0f;
				s.rightTrigger = state.Gamepad.bRightTrigger / 255.0f;

				// Buttons
				s.A = (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;
				s.B = (state.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0;
				s.X = (state.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0;
				s.Y = (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0;
				s.LB = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
				s.RB = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
				s.Back = (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0;
				s.Start = (state.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0;
				s.LS = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0;
				s.RS = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0;
				s.Up = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0;
				s.Down = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
				s.Left = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0;
				s.Right = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;
			}
			else
			{
				gXInputConnected[i] = false;
				continue;
			}
		}
	}
}

bool ermy::input::gamepad::IsConnected(int controllerIndex)
{
	return gXInputConnected[controllerIndex];
}

const ermy::input::gamepad::GamePadState& ermy::input::gamepad::GetState(int controllerIndex)
{
	return gGamePadStates[controllerIndex];
}

ermy::u8 ermy::input::gamepad::GetControllerBatteryLevel(int controllerIndex)
{
	return gControllerBatteryLevel[controllerIndex];
}

bool os::Update()
{
	
	ProcessKeyboardAndMouseRAW();
	ProcessGamepadsXINPUT();

	MSG msg = {};

	//Process buffered raw input messages

	for (int i = 0; i < 4; ++i)
	{
		if (::PeekMessage(&msg, gMainWindow, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	return true;
}

int gMouseGlobalX = 0;
int gMouseGlobalY = 0;

glm::ivec2 ermy::input::mouse::GetCurrentPosition()
{
	POINT currentPos;
	GetCursorPos(&currentPos);
	ScreenToClient(gMainWindow, &currentPos);
	return { currentPos.x, currentPos.y };
}

glm::vec2 ermy::input::mouse::GetDeltaPosition()
{
	//POINT currentPos;
	//GetCursorPos(&currentPos);
	//MOUSEMOVEPOINT mmptIn;
	//mmptIn.x = currentPos.x;
	//mmptIn.y = currentPos.y;
	//mmptIn.time = GetTickCount();

	//// Buffer for up to 64 mouse points
	//MOUSEMOVEPOINT mmptOut[64];
	//UINT pointsReturned = 0;

	//// Call GetMouseMovePointsEx with high-resolution points
	//DWORD result = GetMouseMovePointsEx(
	//	sizeof(MOUSEMOVEPOINT),
	//	&mmptIn,
	//	mmptOut,
	//	64,
	//	GMMP_USE_HIGH_RESOLUTION_POINTS
	//);

	//if (result == 0) { // Success
	//	pointsReturned = 64; // Full buffer returned
	//}
	//else if (result == (DWORD)-1) {
	//	// Error occurred
	//	//char buffer[256];
	//	//sprintf_s(buffer, "GetMouseMovePointsEx failed: %lu\n", GetLastError());
	//	//OutputDebugStringA(buffer);
	//}
	//else {
	//	pointsReturned = result; // Partial buffer returned
	//}

	//if (pointsReturned < 2)
	//{
	//	return { 0, 0 };
	//}

	//MOUSEMOVEPOINT* latest = &mmptOut[0];
	//MOUSEMOVEPOINT* previous = &mmptOut[1];

	//float deltaX = (latest->x - previous->x) / 65536.0f;
	//float deltaY = (latest->y - previous->y) / 65536.0f;

	//return { deltaX, deltaY };

	return { (float)mouseRawDeltaX / 4096.0f, (float)mouseRawDeltaY / 4096.0f };
}

int _ermyKeyCodeToVK(input::keyboard::KeyCode keyCode)
{
	int vkeys[] = { 0,
		'A'
		,'B'
		,'C'
		,'D'
		,'E'
		,'F'
		,'G'
		,'H'
		,'I'
		,'J'
		,'K'
		,'L'
		,'M'
		,'N'
		,'O'
		,'P'
		,'Q'
		,'R'
		,'S'
		,'T'
		,'U'
		,'V'
		,'W'
		,'X'
		,'Y'
		,'Z'
		,VK_NUMPAD0
		,VK_NUMPAD1
		,VK_NUMPAD2
		,VK_NUMPAD3
		,VK_NUMPAD4
		,VK_NUMPAD5
		,VK_NUMPAD6
		,VK_NUMPAD7
		,VK_NUMPAD8
		,VK_NUMPAD9
		,VK_SPACE
		,VK_RETURN
		,VK_BACK
		,VK_TAB
		,VK_LSHIFT
		,VK_LCONTROL
		,VK_LMENU
		,VK_LWIN
		,VK_RSHIFT
		,VK_RCONTROL
		,VK_RMENU
		,VK_RWIN
		,VK_LEFT
		,VK_RIGHT
		,VK_UP
		,VK_DOWN
		,VK_INSERT
		,VK_DELETE
		,VK_HOME
		,VK_END
		,VK_PRIOR
		,VK_NEXT
		,VK_SCROLL
		,VK_SNAPSHOT
		,VK_PAUSE
		,VK_F1
		,VK_F2
		,VK_F3
		,VK_F4
		,VK_F5
		,VK_F6
		,VK_F7
		,VK_F8
		,VK_F9
		,VK_F10
		,VK_F11
		,VK_F12
		,VK_F13
		,VK_F14
		,VK_F15
		,VK_ESCAPE
	};

	return vkeys[(int)keyCode];
}

bool input::keyboard::IsKeyDown(input::keyboard::KeyCode keyCode)
{
	//map vkey to KeyCode

	return GetAsyncKeyState(_ermyKeyCodeToVK(keyCode)) & 0x8000;
}

bool input::keyboard::IsKeyPressed(input::keyboard::KeyCode keyCode)
{
	return GetAsyncKeyState(_ermyKeyCodeToVK(keyCode)) & 0x1;
}

bool input::keyboard::IsKeyReleased(input::keyboard::KeyCode keyCode)
{
	return false;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))
		return true;

	switch (message)
	{
	case WM_INPUT_DEVICE_CHANGE:
	{
		int c = 42;
	}
	break;
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	{
		// Extract the mouse position from LPARAM
		gMouseGlobalX = GET_X_LPARAM(lParam);
		gMouseGlobalY = GET_Y_LPARAM(lParam);
		break;
	}
	case WM_PAINT:
		break;
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
	{
		bool alt = (::GetAsyncKeyState(VK_MENU) & 0x8000) != 0;

		switch (wParam)
		{
		case VK_ESCAPE:
			//gApplication->OnApplicationClose();
			::PostQuitMessage(0);
			break;
		default:
			break;

		}
	}
	break;

	case WM_SYSCHAR:
		break;
		break;
	case WM_SIZE:
	{
		UINT width = LOWORD(lParam);
		UINT height = HIWORD(lParam);

		//llri::swapchain::resize(width, height);
	}
	break;
	case WM_DESTROY:
		gMainWindow = nullptr;
		GetApplication().OnApplicationClose();
		::PostQuitMessage(0);
		break;
	default:
		return ::DefWindowProcW(hwnd, message, wParam, lParam);
	}

	return 0;
}

void* os::GetNativeWindowHandle()
{
	return gMainWindow;
}
void* os::GetAppInstanceHandle()
{
	return gWinSystemInstance;
}

ermy::i64 os::GetCurrentTimestamp()
{
	LARGE_INTEGER timestamp;
	QueryPerformanceCounter(&timestamp);

	return timestamp.QuadPart;
}

namespace ermy::os_utils
{
	struct MappedFileInfoWin
	{
		HANDLE hFile = 0;
		LARGE_INTEGER fileSize = {};
		HANDLE hMapping = 0;
		LPVOID pData = 0;
	};

	MappedFileHandle MapFileReadOnly(const char* pathUtf8)
	{
		MappedFileInfoWin* m = new MappedFileInfoWin();

		m->hFile = CreateFileA(
			pathUtf8,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,                           // Default security
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL                            // No template
		);

		if (m->hFile == INVALID_HANDLE_VALUE) {
			return m;
		}

		if (!GetFileSizeEx(m->hFile, &m->fileSize)) {
			CloseHandle(m->hFile);
			return m;
		}

		HANDLE hMapping = CreateFileMappingA(
			m->hFile,
			NULL,                           // Default security
			PAGE_READONLY,                  // Read-only access
			0,                              // High-order size
			0,                              // Low-order size
			NULL                            // No name for the mapping
		);

		if (!hMapping) {
			CloseHandle(m->hFile);
			return m;
		}

		m->pData = MapViewOfFile(
			hMapping,
			FILE_MAP_READ,
			0,                              // High-order offset
			0,                              // Low-order offset
			m->fileSize.QuadPart            // Whole file
		);

		if (!m->pData) {
			CloseHandle(m->hMapping);
			CloseHandle(m->hFile);
			return m;
		}

		return m;
	}

	MappedFileHandle MapFileWrite(const char* pathUtf8, u64 filesize)
	{
		MappedFileInfoWin* m = new MappedFileInfoWin();

		// Open the file for writing
		m->hFile = CreateFileA(
			pathUtf8,
			GENERIC_READ | GENERIC_WRITE,  // Read and write access
			0,                              // No sharing
			NULL,                           // Default security
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL                            // No template
		);

		if (m->hFile == INVALID_HANDLE_VALUE) {
			return m;
		}

		// Set the file size
		LARGE_INTEGER size;
		size.QuadPart = filesize;
		if (!SetFilePointerEx(m->hFile, size, NULL, FILE_BEGIN) || !SetEndOfFile(m->hFile)) {
			CloseHandle(m->hFile);
			return m;
		}

		// Create a file mapping object
		m->hMapping = CreateFileMappingA(
			m->hFile,
			NULL,                           // Default security
			PAGE_READWRITE,                 // Read/write access
			size.HighPart,                  // High-order size
			size.LowPart,                   // Low-order size
			NULL                            // No name for the mapping
		);

		if (!m->hMapping) {
			CloseHandle(m->hFile);
			return m;
		}

		// Map the file into memory
		m->pData = MapViewOfFile(
			m->hMapping,
			FILE_MAP_WRITE,                 // Write access
			0,                              // High-order offset
			0,                              // Low-order offset
			filesize                        // Whole file
		);

		if (!m->pData) {
			CloseHandle(m->hMapping);
			CloseHandle(m->hFile);
			return m;
		}

		return m;
	}

	void* GetPointer(MappedFileHandle mfile)
	{
		MappedFileInfoWin* m = (MappedFileInfoWin*)mfile;
		return m->pData;
	}

	void CloseMappedFile(MappedFileHandle mfile)
	{
		MappedFileInfoWin* m = (MappedFileInfoWin*)mfile;
		UnmapViewOfFile(m->pData);
		CloseHandle(m->hMapping);
		CloseHandle(m->hFile);
		delete m;
	}

	void SetNativeWindowTitle(const char* title)
	{
		if (gMainWindow)
		{
			wchar_t wTitle[256];
			::os::UTF8ToWCS(title, wTitle);
			SetWindowTextW(gMainWindow, wTitle);
		}
	}

	const char* GetOSName()
	{
		return "Windows";
	}
}
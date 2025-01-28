#pragma once
#include "win_utils.h"
#include "application.h"

HWND gMainWindow = nullptr;
HINSTANCE gWinSystemInstance = nullptr;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

constexpr const wchar_t* winClassName = L"ErmyWindow";


void* os::CreateNativeWindow()
{
	gWinSystemInstance = GetModuleHandleA(nullptr);
	auto& config = GetApplication().staticConfig;
	auto& winCfg = GetApplication().staticConfig.window;

	const char* utf8WinCaption = config.GetWindowTitle().c_str();
	bool is_maximized = winCfg.mode != ermy::Application::StaticConfig::WindowConfig::WindowMode::Windowed;

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

	HWND hWnd = ::CreateWindowExW(
		0,
		winClassName,
		winTitleBuff,
		WS_OVERLAPPEDWINDOW,
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

	gMainWindow = hWnd;

	return static_cast<void*>(gMainWindow);
}

bool os::Update()
{
	MSG msg = {};

	if (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	os::Sleep(1);
	return true;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))
		return true;

	switch (message)
	{
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
		//gApplication->OnApplicationClose();
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
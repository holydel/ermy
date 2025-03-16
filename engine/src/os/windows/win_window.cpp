#pragma once
#include "win_utils.h"
#include "application.h"
#include <windowsx.h>
#include <ermy_input.h>
using namespace ermy;

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

	if (winCfg.supportTransparent)
	{
		exFlags |= WS_EX_LAYERED | WS_EX_TRANSPARENT;
	}

	HWND hWnd = ::CreateWindowExW(
		exFlags,
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

	return true;
}

int gMouseGlobalX = 0;
int gMouseGlobalY = 0;

glm::ivec2 ermy::input::mouse::GetCurrentPosition()
{
	return { gMouseGlobalX ,gMouseGlobalY };
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))
		return true;

	switch (message)
	{
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
		if(gMainWindow)
		{
			wchar_t wTitle[256];
			::os::UTF8ToWCS(title, wTitle);
			SetWindowTextW(gMainWindow, wTitle);
		}
	}
}
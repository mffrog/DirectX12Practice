#pragma once
#include <Windows.h>
class Window {
public:
	static Window& Get();
	bool Init(HINSTANCE hInstance, WNDPROC proc, int width, int height, const wchar_t* windowTitle, int nCmdShow);
	bool WindowDestroyed() const { return windowDestroyed; }
	void UpdateMessage();
	HWND GetWindowHandle() const { return hwnd; }
private:
	Window() = default;
	~Window() = default;
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	HWND hwnd;
	bool windowDestroyed = false;
	bool isInitialized = false;
};
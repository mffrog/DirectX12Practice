#include "Window.h"
const wchar_t windowClassNmae[] = L"DX12Practice";

Window& Window::Get() {
	static Window instance;
	return instance;
}

bool Window::Init(HINSTANCE hInstance, WNDPROC proc, int width, int height, const wchar_t* windowTitle, int nCmdShow) {
	if (isInitialized) {
		return true;
	}
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = proc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW);
	wc.lpszClassName = windowClassNmae;
	RegisterClassEx(&wc);

	RECT windowRect = { 0,0,width, height };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	hwnd = CreateWindow(windowClassNmae, windowTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, nullptr, nullptr, hInstance, nullptr);

	ShowWindow(hwnd, nCmdShow);
	windowDestroyed = false;
	isInitialized = true;
	return true;
}

void Window::UpdateMessage() {
	//メッセージ処理
	MSG msg;
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_QUIT) {
			windowDestroyed = true;
		}
	}
}

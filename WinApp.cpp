#include "WinApp.h"

LRESULT CALLBACK WindowProc(
    HWND hwnd,
    UINT msg,
    WPARAM wparam,
    LPARAM lparam
) {
    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}

void WinApp::Initialize() {

    WNDCLASS wc{};

    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = L"CG2WindowClass";
    wc.hInstance = GetModuleHandle(nullptr);

    RegisterClass(&wc);

    RECT wrc = { 0, 0, kClientWidth, kClientHeight };

    AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

    hwnd_ = CreateWindow(
        wc.lpszClassName,
        L"CG2",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        wrc.right - wrc.left,
        wrc.bottom - wrc.top,
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr
    );

    ShowWindow(hwnd_, SW_SHOW);
}

void WinApp::ProcessMessage() {
    if (PeekMessage(&msg_, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg_);
        DispatchMessage(&msg_);
    }
}

bool WinApp::IsEndGame() {
    return msg_.message == WM_QUIT;
}
#pragma once
#include <Windows.h>
#include <cstdint>

class WinApp {
public:
	// クライアント領域サイズ
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

	~WinApp();

	void Initialize();
	void ProcessMessage();
	bool IsEndGame();

	HWND GetHwnd() const { return hwnd_; }

private:
	HWND hwnd_ = nullptr;
	MSG msg_{};
};



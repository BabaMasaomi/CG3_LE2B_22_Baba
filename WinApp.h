#pragma once
#include <Windows.h>
#include <cstdint>

class WinApp {
public:
	// クライアント領域サイズ
	static const int32_t kClientWidth_ = 1280;
	static const int32_t kClientHeight_ = 720;

	~WinApp();

	void Initialize();
	void ProcessMessage();
	bool IsEndGame();

	// ゲッター
	HWND GetHwnd() const { return hwnd_; }

private:
	HWND hwnd_ = nullptr;
	MSG msg_{};
};



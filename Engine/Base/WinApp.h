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

	void AddWheelDelta(int delta);

	// ゲッター
	HWND GetHwnd() const { return hwnd_; }

	int GetWheelDelta() const { return wheelDelta_; }
	void ResetWheelDelta() { wheelDelta_ = 0; }
	static WinApp* GetInstance() { return instance_; }


private:
	HWND hwnd_ = nullptr;
	MSG msg_{};

	int wheelDelta_ = 0;

	static WinApp* instance_;
};



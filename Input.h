#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <wrl.h>

// libのリンク
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

class Input {
public:
	void Initialize(HINSTANCE hInstance, HWND hwnd);
	void Update();

	bool PushKey(BYTE keyNumber);		// Push
	bool ReleaseKey(BYTE keyNumber);	// Release
	bool TriggerKey(BYTE keyNumber);	// Trigger

private:
	Microsoft::WRL::ComPtr<IDirectInput8> directInput_ = nullptr;
	Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard_ = nullptr;

	BYTE key_[256] = {};
	BYTE preKey_[256] = {};
};
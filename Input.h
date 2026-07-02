#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include "struct.h"
#include <wrl.h>

// libのリンク
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

class Input {
public:
	void Initialize(HINSTANCE hInstance, HWND hwnd);
	void Update();

	// キーボードの入力関数
	bool PushKey(BYTE keyNumber);		// Push
	bool TriggerKey(BYTE keyNumber);	// Trigger
	bool ReleaseKey(BYTE keyNumber);	// Release

	// マウスの入力関数
	// Push
	bool PushMouseLeft();
	bool PushMouseRight();
	bool PushMouseMiddle();
	// Trigger
	bool TriggerMouseLeft();
	bool TriggerMouseRight();
	bool TriggerMouseMiddle();
	// Release
	bool ReleaseMouseLeft();
	bool ReleaseMouseRight();
	bool ReleaseMouseMiddle();
	// マウスホイールの入力関数
	void AddWheelDelta(int delta);

	// ゲッター
	Vector2 GetMouseDelta() const { return mouseDelta_; }
	Vector2 GetMousePos() const { return Vector2(float(mousePos_.x), float(mousePos_.y)); }
	int GetWheelDelta() const { return wheelDelta_; }

private:
	// キーボード入力用
	Microsoft::WRL::ComPtr<IDirectInput8> directInput_ = nullptr;
	Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard_ = nullptr;

	BYTE key_[256] = {};
	BYTE preKey_[256] = {};

	// マウス入力用
	POINT mousePos_ = {};
	POINT preMousePos_ = {};
	// ウィンドウハンドル
	HWND hwnd_ = nullptr;
	Vector2 mouseDelta_ = {};
	// マウスホイールの回転量
	int wheel_ = 0;
	int wheelDelta_ = 0;
	// マウスのボタンの状態を保存する配列
	bool mouseButton_[3] = {};
	bool preMouseButton_[3] = {};
	// マウスの移動量を保存する変数
	Vector3 target_ = {};
};
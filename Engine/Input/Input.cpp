#include "Input.h"
#include "WinApp.h"
#include <cassert>

/*-------------------- 初期化 --------------------*/
void Input::Initialize(HINSTANCE hInstance, HWND hwnd) {
	// DirectInputオブジェクトの生成
	HRESULT result = DirectInput8Create(
		hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)directInput_.GetAddressOf(), nullptr);
	assert(SUCCEEDED(result));

	// キーボードデバイスの生成
	result = directInput_.Get()->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
	assert(SUCCEEDED(result));

	// 入力データ形式のセット
	result = keyboard_.Get()->SetDataFormat(&c_dfDIKeyboard);	// 標準形式
	assert(SUCCEEDED(result));

	// 排他制御レベルのセット
	result = keyboard_.Get()->SetCooperativeLevel(
		hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

	// キーボードの状態を初期化
	memset(key_, 0, sizeof(key_));
	memset(preKey_, 0, sizeof(preKey_));

	// マウス用に変数を取得
	hwnd_ = hwnd;
}

/*-------------------- 更新 --------------------*/
void Input::Update() {
	// 前フレームのキーボードの入力状態を保存
	memcpy(preKey_, key_, sizeof(key_));
	// キーボード情報の取得開始
	keyboard_.Get()->Acquire();
	// 全キーボードの入力状態を取得する
	keyboard_.Get()->GetDeviceState(sizeof(key_), key_);

	// 前フレームのマウスの座標を保存
	preMousePos_ = mousePos_;
	// カーソルの位置を取得
	GetCursorPos(&mousePos_);
	// ウィンドウクライアント座標に変換
	ScreenToClient(hwnd_, &mousePos_);
	// マウスの移動量を計算
	mouseDelta_.x = float(mousePos_.x - preMousePos_.x);
	mouseDelta_.y = float(mousePos_.y - preMousePos_.y);

	// 前フレームのマウスの入力状態を保存
	preMouseButton_[0] = mouseButton_[0];
	preMouseButton_[1] = mouseButton_[1];
	preMouseButton_[2] = mouseButton_[2];
	// マウスの入力状態を取得
	mouseButton_[0] = GetAsyncKeyState(VK_LBUTTON) & 0x8000;
	mouseButton_[1] = GetAsyncKeyState(VK_RBUTTON) & 0x8000;
	mouseButton_[2] = GetAsyncKeyState(VK_MBUTTON) & 0x8000;

	// マウスホイールの回転量を初期化
	wheelDelta_ = WinApp::GetInstance()->GetWheelDelta();
	WinApp::GetInstance()->ResetWheelDelta();
}

/*-------------------- キーボードPush判定 --------------------*/
bool Input::PushKey(BYTE keyNumber) {
	return key_[keyNumber];
}

/*-------------------- キーボードTrigger判定 --------------------*/
bool Input::TriggerKey(BYTE keyNumber) {
	return key_[keyNumber] && !preKey_[keyNumber];
}

/*-------------------- キーボードRelease判定 --------------------*/
bool Input::ReleaseKey(BYTE keyNumber) {
	return !key_[keyNumber] && preKey_[keyNumber];
}

/*-------------------- マウスPush判定 --------------------*/
bool Input::PushMouseLeft() {
	return mouseButton_[0];
}
bool Input::PushMouseRight() {
	return mouseButton_[1];
}
bool Input::PushMouseMiddle() {
	return mouseButton_[2];
}

/*-------------------- マウスTrigger判定 --------------------*/
bool Input::TriggerMouseLeft() {
	return mouseButton_[0] && !preMouseButton_[0];
}
bool Input::TriggerMouseRight() {
	return mouseButton_[1] && !preMouseButton_[1];
}
bool Input::TriggerMouseMiddle() {
	return mouseButton_[2] && !preMouseButton_[2];
}

/*-------------------- マウスRelease判定 --------------------*/
bool Input::ReleaseMouseLeft() {
	return !mouseButton_[0] && preMouseButton_[0];
}
bool Input::ReleaseMouseRight() {
	return !mouseButton_[1] && preMouseButton_[1];
}
bool Input::ReleaseMouseMiddle() {
	return !mouseButton_[2] && preMouseButton_[2];
}

/*-------------------- マウスホイールの回転量を取得 --------------------*/
void Input::AddWheelDelta(int delta) {
	wheelDelta_ += delta;
}
#include "Input.h"
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
}

/*-------------------- 更新 --------------------*/
void Input::Update() {
	// 前フレームの入力状態を保存
	memcpy(preKey_, key_, sizeof(key_));
	// キーボード情報の取得開始
	keyboard_.Get()->Acquire();
	// 全キーボードの入力状態を取得する
	keyboard_.Get()->GetDeviceState(sizeof(key_), key_);
}

/*-------------------- Push判定 --------------------*/
bool Input::PushKey(BYTE keyNumber) {
	return key_[keyNumber];
}

/*-------------------- Release判定 --------------------*/
bool Input::ReleaseKey(BYTE keyNumber){
	return !key_[keyNumber] && preKey_[keyNumber];
}

/*-------------------- Trigger判定 --------------------*/
bool Input::TriggerKey(BYTE keyNumber) {
	return key_[keyNumber] && !preKey_[keyNumber];
}
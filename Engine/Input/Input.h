#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include "struct.h"
#include <wrl.h>

// libのリンク
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

namespace Engine {

	class Input {
	public:
		/*==================== メンバ関数の宣言 ====================*/
		/// <summary>
		/// Inputの初期化
		/// </summary>
		/// <param name="hInstance">インスタンスハンドル</param>
		/// <param name="hwnd">ウィンドウハンドル</param>
		void Initialize(HINSTANCE hInstance, HWND hwnd);
		/// <summary>
		/// Inputの更新
		/// </summary>
		void Update();

		/// <summary>
		/// キーが押されているかを判定する
		/// </summary>
		/// <param name="keyNumber">キーの番号</param>
		/// <returns>押されている場合はtrue、そうでない場合はfalse</returns>
		bool PushKey(BYTE keyNumber);
		/// <summary>
		/// キーが押された瞬間かを判定する
		/// </summary>
		/// <param name="keyNumber">キーの番号</param>
		/// <returns>押された瞬間の場合はtrue、そうでない場合はfalse</returns>
		bool TriggerKey(BYTE keyNumber);
		/// <summary>
		/// キーが離された瞬間かを判定する
		/// </summary>
		/// <param name="keyNumber">キーの番号</param>
		/// <returns>離された瞬間の場合はtrue、そうでない場合はfalse</returns>
		bool ReleaseKey(BYTE keyNumber);

		/// <summary>
		/// マウスの左ボタンが押されているかを判定する
		/// </summary>
		/// <returns>押されている場合はtrue、そうでない場合はfalse</returns>
		bool PushMouseLeft();
		/// <summary>
		/// マウスの右ボタンが押されているかを判定する
		/// </summary>
		/// <returns>押されている場合はtrue、そうでない場合はfalse</returns>
		bool PushMouseRight();
		/// <summary>
		/// マウスの中央ボタンが押されているかを判定する
		/// </summary>
		/// <returns>押されている場合はtrue、そうでない場合はfalse</returns>
		bool PushMouseMiddle();

		/// <summary>
		/// マウスの左ボタンが押された瞬間かを判定する
		/// </summary>
		/// <returns>押された瞬間の場合はtrue、そうでない場合はfalse</returns>
		bool TriggerMouseLeft();
		/// <summary>
		/// マウスの右ボタンが押された瞬間かを判定する
		/// </summary>
		/// <returns>押された瞬間の場合はtrue、そうでない場合はfalse</returns>
		bool TriggerMouseRight();
		/// <summary>
		/// マウスの中央ボタンが押された瞬間かを判定する
		/// </summary>
		/// <returns>押された瞬間の場合はtrue、そうでない場合はfalse</returns>
		bool TriggerMouseMiddle();

		/// <summary>
		/// マウスの左ボタンが離された瞬間かを判定する
		/// </summary>
		/// <returns>離された瞬間の場合はtrue、そうでない場合はfalse</returns>
		bool ReleaseMouseLeft();
		/// <summary>
		/// マウスの右ボタンが離された瞬間かを判定する
		/// </summary>
		/// <returns>離された瞬間の場合はtrue、そうでない場合はfalse</returns>
		bool ReleaseMouseRight();
		/// <summary>
		/// マウスの中央ボタンが離された瞬間かを判定する
		/// </summary>
		/// <returns>離された瞬間の場合はtrue、そうでない場合はfalse</returns>
		bool ReleaseMouseMiddle();

		/// <summary>
		/// マウスホイールの回転量を追加する
		/// </summary>
		/// <param name="delta">マウスホイールの回転量</param>
		void AddWheelDelta(int delta);

		/*==================== アクセッサ ====================*/
		// ゲッター
		Vector2 GetMouseDelta() const { return mouseDelta_; }
		Vector2 GetMousePos() const { return Vector2(float(mousePos_.x), float(mousePos_.y)); }
		int GetWheelDelta() const { return wheelDelta_; }

	private:
		/*==================== メンバ変数 ====================*/
		// キーボード入力用
		Microsoft::WRL::ComPtr<IDirectInput8> directInput_ = nullptr;
		Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard_ = nullptr;

		// DirectInputDevice8の状態を保存する配列
		// キーの状態を保存する配列
		BYTE key_[256] = {};
		// 前回のキーの状態を保存する配列
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
}
#pragma once
#include <Windows.h>
#include <cstdint>

namespace Engine {

	class WinApp {
	public:
		/*==================== メンバ変数 ====================*/
		// クライアント領域サイズ
		// ウィンドウの幅
		static const int32_t kClientWidth_ = 1280;
		// ウィンドウの高さ
		static const int32_t kClientHeight_ = 720;

		/*==================== メンバ関数とデストラクタの宣言 ====================*/
		// デストラクタ
		~WinApp();

		/// <summary>
		/// WinAppの初期化
		/// </summary>
		void Initialize();
		/// <summary>
		/// メッセージの処理
		/// </summary>
		void ProcessMessage();
		/// <summary>
		/// ゲーム終了判定
		/// </summary>
		/// <returns>ゲームが終了している場合はtrue、それ以外はfalse</returns>
		bool IsEndGame();
		/// <summary>
		/// ホイールの回転量を追加
		/// </summary>
		/// <param name="delta">ホイールの回転量</param>
		void AddWheelDelta(int delta);

		/*==================== アクセッサ ====================*/
		// ゲッター
		HWND GetHwnd() const { return hwnd_; }
		int GetWheelDelta() const { return wheelDelta_; }
		void ResetWheelDelta() { wheelDelta_ = 0; }
		static WinApp* GetInstance() { return instance_; }


	private:
		/*==================== メンバ変数 ====================*/
		// ウィンドウプロシージャ
		HWND hwnd_ = nullptr;
		// メッセージ
		MSG msg_{};
		// ホイールの回転量
		int wheelDelta_ = 0;
		// ウィンドウアプリケーションのインスタンス
		static WinApp* instance_;
	};
}
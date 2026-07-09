#pragma once
#include <Windows.h>
#include <cstdint>

class WinApp {
public:
	// クライアント領域サイズ
	// ウィンドウの幅
	static const int32_t kClientWidth_ = 1280;
	// ウィンドウの高さ
	static const int32_t kClientHeight_ = 720;

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

	// ゲッター
	HWND GetHwnd() const { return hwnd_; }
	int GetWheelDelta() const { return wheelDelta_; }
	void ResetWheelDelta() { wheelDelta_ = 0; }
	static WinApp* GetInstance() { return instance_; }


private:
	// ウィンドウプロシージャ
	HWND hwnd_ = nullptr;
	// メッセージ
	MSG msg_{};
	// ホイールの回転量
	int wheelDelta_ = 0;
	// ウィンドウアプリケーションのインスタンス
	static WinApp* instance_;
};



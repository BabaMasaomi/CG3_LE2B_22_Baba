#pragma once
#include "externals/imgui/imgui.h"

namespace Engine {
#ifdef USE_IMGUI
	class WinApp;
	class DirectXCommon;

	class ImGuiManager {
	public:
		/*==================== メンバ関数の宣言 ====================*/
		/// <summary>
		/// ImGuiManagerの初期化
		/// </summary>
		/// <param name="winApp">WinAppのインスタンス</param>
		/// <param name="dxCommon">DirectXCommonのインスタンス</param>
		void Initialize(WinApp* winApp, DirectXCommon* dxCommon);
		/// <summary>
		/// ImGuiの描画を開始する
		/// </summary>
		void Begin();
		/// <summary>
		/// ImGuiの描画を終了する
		/// </summary>
		void End();
		/// <summary>
		/// ImGuiの描画を行う
		/// </summary>
		void Draw();
		/// <summary>
		/// ImGuiManagerの終了処理
		/// </summary>
		void Finalize();

	private:
		/*==================== メンバ変数 ====================*/
		// ウィンドウアプリケーションのインスタンス
		WinApp* winApp_ = nullptr;
		// DirectXCommonのインスタンス
		DirectXCommon* dxCommon_ = nullptr;
	};
#endif
}
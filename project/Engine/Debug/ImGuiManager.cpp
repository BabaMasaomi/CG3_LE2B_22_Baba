#ifdef USE_IMGUI
#include "ImGuiManager.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

namespace Engine {

	/*-------------------- 初期化 --------------------*/
	void ImGuiManager::Initialize(WinApp* winApp, DirectXCommon* dxCommon) {
		// メンバ変数に引数をセット
		winApp_ = winApp;
		dxCommon_ = dxCommon;

		// ImGuiの初期化
		IMGUI_CHECKVERSION();
		// ImGuiのコンテキストを作成
		ImGui::CreateContext();
		// ImGuiのスタイルを設定
		ImGui::StyleColorsDark();
		//	ImGuiのバックエンドを初期化
		ImGui_ImplWin32_Init(winApp->GetHwnd());
		ImGui_ImplDX12_Init(dxCommon->GetDevice(),
			dxCommon->GetSwapChainDesc().BufferCount,
			dxCommon->GetrtvDesc().Format,
			dxCommon->GetSrvDescriptorHeap(),
			dxCommon->GetSrvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(),
			dxCommon->GetSrvDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
		// フォントのビルド
		ImGuiIO& io = ImGui::GetIO();
		// フォントの設定
		io.Fonts->Build();
	}

	/*-------------------- フレーム開始 --------------------*/
	void ImGuiManager::Begin() {
		// ImGuiのフレームを開始する
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	/*-------------------- 描画処理 --------------------*/
	void ImGuiManager::Draw() {
		// ImGuiの描画を行う
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon_->GetCommandList());
	}

	/*-------------------- フレーム終了 --------------------*/
	void ImGuiManager::End() {
		// 描画コマンドを確定する
		ImGui::Render();
	}

	/*-------------------- 終了処理 --------------------*/
	void ImGuiManager::Finalize() {
		// ImGuiの終了処理
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		// ImGuiのコンテキストを破棄する
		ImGui::DestroyContext();
	}
}
#endif
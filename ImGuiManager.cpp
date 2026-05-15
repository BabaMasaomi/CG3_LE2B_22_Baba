#include "ImGuiManager.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

/*-------------------- 初期化 --------------------*/
void ImGuiManager::Initialize(WinApp* winApp, DirectXCommon* dxCommon) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp->GetHwnd());
	ImGui_ImplDX12_Init(dxCommon->GetDevice(),
		dxCommon->GetSwapChainDescBufferCount(),
		dxCommon->GetrtvDescFormat(),
		dxCommon->GetsrvDescriptorHeap(),
		dxCommon->GetsrvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(),
		dxCommon->GetsrvDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->Build();
}

/*-------------------- フレーム開始 --------------------*/
void ImGuiManager::Begin() {
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

/*-------------------- 描画処理 --------------------*/
void ImGuiManager::Draw() {
	ImGui_ImplDX12_RenderDrawData(
		ImGui::GetDrawData(),
		dxCommon_->GetCommandList()
	);
}

/*-------------------- フレーム終了 --------------------*/
void ImGuiManager::End() {
	ImGui::Render();
}

/*-------------------- 終了処理 --------------------*/
void ImGuiManager::Finalize() {
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
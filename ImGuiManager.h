#pragma once
#include "externals/imgui/imgui.h"

#ifdef USE_IMGUI
class WinApp;
class DirectXCommon;

class ImGuiManager {
public:
	void Initialize(WinApp* winApp, DirectXCommon* dxCommon);
	void Begin();
	void End();
	void Draw();

	void Finalize();

private:
	WinApp* winApp_ = nullptr;
	DirectXCommon* dxCommon_ = nullptr;
};
#endif
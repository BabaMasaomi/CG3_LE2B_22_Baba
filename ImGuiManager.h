#pragma once

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
    WinApp* winApp_;
    DirectXCommon* dxCommon_;
};
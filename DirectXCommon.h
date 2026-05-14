#pragma once
#include <Windows.h>
#include <cstdint>
// DirectXを使うためのライブラリ
#include <d3d12.h>
#include <dxgi1_6.h>

// libのリンク
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class DirectXCommon {
public:
	~DirectXCommon();

    void Initialize(HWND hwnd);

    void PreDraw();
    void PostDraw();

	// ゲッター
    ID3D12Device* GetDevice() const { return device_; }
    ID3D12GraphicsCommandList* GetCommandList() const { return commandList_; }
    const D3D12_VIEWPORT& GetViewPort() const { return viewPort_; }
    const D3D12_RECT& GetScissorRect() const { return scissorRect_; }

private:
    void InitializeDebugLayer();        // デバッグレイヤーの初期化
    void CreateDevice();                // 1. Factoryの生成、アダプタ選択、D3D12デバイス生成
    void InitializeInfoQueue();         // Debug情報の初期化
    void CreateCommand();               // 2. コマンドキュー、コマンドアロケータ、コマンドリストの生成
    void CreateSwapChain(HWND hwnd);    // 3. SwapChainの生成
    void CreateRenderTargetView();      // 4. レンダーターゲットビューの生成
    void CreateFence();                 // 5. Fenceの生成
	void CreateViewport();              // 描画用のViewPortとScissorRectの生成

#ifdef _DEBUG
	// デバッグレイヤーのコントローラー
    ID3D12Debug1* debugController_ = nullptr;
#endif

    // DXGIファクトリーの生成
    IDXGIFactory7* dxgiFactory_ = nullptr;
    ID3D12Device* device_ = nullptr;

    // コマンドキューを作成
    ID3D12CommandQueue* commandQueue_ = nullptr;
    // コマンドアロケータを作成
    ID3D12CommandAllocator* commandAllocator_ = nullptr;
    // コマンドリストを作成
    ID3D12GraphicsCommandList* commandList_ = nullptr;

    // ViewPortとScissorRectを準備(図形描画用)
	D3D12_VIEWPORT viewPort_ = {};
	D3D12_RECT scissorRect_ = {};

	// スワップチェーンを作成
    IDXGISwapChain4* swapChain_ = nullptr;

    // ディスクリプタヒープを作成
    ID3D12DescriptorHeap* rtvDescriptorHeap_ = nullptr;
    // SwapChainからResourceを持ってくる
    ID3D12Resource* swapChainResources_[2] = {};
    // ディスクリプタの先頭を取得
    D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle_;
    // RTVを2つ作るのでディスクリプタを2つ分用意
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2];

    // Fenceを作成
    ID3D12Fence* fence_ = nullptr;
    uint64_t fenceValue_ = 0;
    HANDLE fenceEvent_ = nullptr;

};
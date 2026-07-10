#pragma once
#include <Windows.h>
#include <cstdint>
#include <wrl.h>
// DirectXを使うためのライブラリ
#include <d3d12.h>
#include <dxgi1_6.h>

// libのリンク
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace Engine {

	// 前方宣言
	class WinApp;

	class DirectXCommon {
	public:
		/*==================== メンバ関数とデストラクタの宣言 ====================*/
		// デストラクタ
		~DirectXCommon();

		/// <summary>
		/// DirectXの初期化
		/// </summary>
		/// <param name="hwnd">ウィンドウハンドル</param>
		/// <param name="width">ウィンドウの幅</param>
		/// <param name="height">ウィンドウの高さ</param>
		void Initialize(HWND hwnd, int32_t width, int32_t height);
		/// <summary>
		/// デバイスの初期化
		/// </summary>
		void InitializeDevice();
		/// <summary>
		/// レンダーターゲットの初期化
		/// </summary>
		/// <param name="hwnd">ウィンドウハンドル</param>
		/// <param name="width">ウィンドウの幅</param>
		/// <param name="height">ウィンドウの高さ</param>
		void InitializeRenderTarget(HWND hwnd, int32_t width, int32_t height);
		/// <summary>
		/// 同期オブジェクトの初期化
		/// </summary>
		void InitializeSynchronization();

		/// <summary>
		/// 描画前の処理
		/// </summary>
		void PreDraw();
		/// <summary>
		/// 描画後の処理
		/// </summary>
		void PostDraw();

		/// <summary>
		/// コマンドを実行
		/// </summary>
		void ExecuteCommandList();
		/// <summary>
		/// GPUを待機
		/// </summary>
		void WaitForGPU();
		/// <summary>
		/// コマンドリストをリセット
		/// </summary>
		void ResetCommandList();

		/*==================== アクセッサ ====================*/
		// ゲッター
		uint32_t GetDescriptorSizeSRV() const { return DescriptorSizeSRV_; }
		uint32_t GetDescriptorSizeRTV() const { return DescriptorSizeRTV_; }
		uint32_t GetDescriptorSizeDSV() const { return DescriptorSizeDSV_; }
		ID3D12Device* GetDevice() const { return device_.Get(); }
		ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }
		const D3D12_VIEWPORT& GetViewPort() const { return viewPort_; }
		const D3D12_RECT& GetScissorRect() const { return scissorRect_; }
		ID3D12DescriptorHeap* GetSrvDescriptorHeap()const { return srvDescriptorHeap_.Get(); }
		DXGI_SWAP_CHAIN_DESC1 GetSwapChainDesc() const { return swapChainDesc_; }
		D3D12_RENDER_TARGET_VIEW_DESC GetrtvDesc() const { return rtvDesc_; }

	private:

		/*==================== メンバ関数の宣言 ====================*/
		/// <summary>
		/// デバッグレイヤーの初期化
		/// </summary>
		void InitializeDebugLayer();
		/// <summary>
		/// 1. D3D12デバイスの生成(Factoryの生成、アダプタ選択、D3D12デバイス生成)
		/// </summary>
		void CreateDevice();
		/// <summary>
		/// Debug情報の初期化
		/// </summary>
		void InitializeInfoQueue();
		/// <summary>
		/// 2. コマンドの生成(コマンドキュー、コマンドアロケータ、コマンドリスト)
		/// </summary>
		void CreateCommand();
		/// <summary>
		/// 3. SwapChainの生成
		/// </summary>
		/// <param name="hwnd">ウィンドウハンドル</param>
		void CreateSwapChain(HWND hwnd);
		/// <summary>
		/// 4. レンダーターゲットビューの生成
		/// </summary>
		void CreateRenderTargetView();
		/// <summary>
		/// DepthStencilTextureResourceを作成
		/// </summary>
		/// <param name="device">D3D12デバイス</param>
		/// <param name="width">テクスチャの幅</param>
		/// <param name="height">テクスチャの高さ</param>
		/// <returns>作成されたDepthStencilTextureResource</returns>
		Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height);
		/// <summary>
		/// DepthStencilViewを作成
		/// </summary>
		void DepthStencilView();
		/// <summary>
		/// 5. Fenceの生成
		/// </summary>
		void CreateFence();
		/// <summary>
		/// ViewPortとScissorRectの生成
		/// </summary>
		void CreateViewport();

		/*==================== メンバ変数 ====================*/
		// ウィンドウアプリケーションのポインタ
		WinApp* winApp_ = nullptr;

#ifdef _DEBUG
		// デバッグレイヤーのコントローラー
		Microsoft::WRL::ComPtr<ID3D12Debug1> debugController_ = nullptr;
#endif

		// DXGIファクトリーの生成
		Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_ = nullptr;
		// D3D12デバイスの生成
		Microsoft::WRL::ComPtr<ID3D12Device> device_ = nullptr;

		// コマンドキューを作成
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_ = nullptr;
		// コマンドアロケータを作成
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_ = nullptr;
		// コマンドリストを作成
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;

		// ViewPortとScissorRectを準備(図形描画用)
		// ViewPortは描画先の矩形領域を設定する構造体
		D3D12_VIEWPORT viewPort_ = {};
		// ScissorRectは描画先の矩形領域を設定する構造体
		D3D12_RECT scissorRect_ = {};

		// スワップチェーンを作成
		Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_ = nullptr;

		// ディスクリプタヒープを作成
		// RTV用
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_ = nullptr;
		// SRV用
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_ = nullptr;

		// DepthStencilTextureResource
		Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_ = nullptr;
		// DepthStencilView
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandleCPU_{};
		// DSV用のディスクリプタヒープ
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_ = nullptr;

		// スワップチェーンの設定
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc_{};
		// SwapChainからResourceを持ってくる
		Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources_[2] = {};
		// RTV設定
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_{};
		// ディスクリプタの先頭を取得
		D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle_;
		// RTVを2つ作るのでディスクリプタを2つ分用意
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2];

		// Fenceを作成
		Microsoft::WRL::ComPtr<ID3D12Fence> fence_ = nullptr;
		// Fenceの値を保持する変数
		uint64_t fenceValue_ = 0;
		// FenceのSignalを待つためのEventを作成
		HANDLE fenceEvent_ = nullptr;

		// descriptorSizeを取得
		// SRV,RTV,DSVのDescriptorSizeを取得しておく
		// DescriptorSizeは、DescriptorHeapの中の1つのDescriptorのサイズを表す
		uint32_t DescriptorSizeSRV_ = 0;
		// RTVとDSVはSRVと同じサイズなので、SRVのサイズを取得しておけば良い
		uint32_t DescriptorSizeRTV_ = 0;
		// DSVのサイズは、RTVと同じサイズなので、RTVのサイズを取得しておけば良い
		uint32_t DescriptorSizeDSV_ = 0;

	};
}
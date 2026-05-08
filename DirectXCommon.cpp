#include "DirectXCommon.h"
#include "WinApp.h"
#include <cassert>

void DirectXCommon::Initialize(HWND hwnd) {
	CreateDevice();
	CreateCommand();
	CreateSwapChain(hwnd);
	CreateRenderTargetView();
	CreateFence();
}

// 1.Factoryの生成、アダプタ選択、D3D12デバイス生成
void DirectXCommon::CreateDevice() {
	HRESULT hr;

	hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
	assert(SUCCEEDED(hr));

	IDXGIAdapter4* useAdapter = nullptr;

	for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i, 
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != 
		DXGI_ERROR_NOT_FOUND; ++i) {
		// アダプタの情報を取得
		DXGI_ADAPTER_DESC3 adapterDesc{};
		useAdapter->GetDesc3(&adapterDesc);

		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			break;
		}
	}

	hr = D3D12CreateDevice(useAdapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device_));
	assert(SUCCEEDED(hr));
}

// 2.コマンドキュー、コマンドアロケータ、コマンドリストの生成
void DirectXCommon::CreateCommand() {
	HRESULT hr;

	D3D12_COMMAND_QUEUE_DESC queueDesc{};

	hr = device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue_));
	assert(SUCCEEDED(hr));

	hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
	assert(SUCCEEDED(hr));

	hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_, nullptr, IID_PPV_ARGS(&commandList_));
	assert(SUCCEEDED(hr));
}

// 3.スワップチェーンの生成
void DirectXCommon::CreateSwapChain(HWND hwnd) {
	HRESULT hr;

	DXGI_SWAP_CHAIN_DESC1 desc{};

	desc.Width = WinApp::kClientWidth;
	desc.Height = WinApp::kClientHeight;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = 2;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	hr = dxgiFactory_->CreateSwapChainForHwnd(commandQueue_, hwnd, &desc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(&swapChain_));
	assert(SUCCEEDED(hr));
}

// 4.レンダーターゲットビューの生成
void DirectXCommon::CreateRenderTargetView() {
	HRESULT hr;

	// DescriptorHeap作成
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NumDescriptors = 2;

	hr = device_->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvDescriptorHeap_));
	assert(SUCCEEDED(hr));

	// SwapChainからBackBuffer取得
	hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(&swapChainResources_[0]));
	assert(SUCCEEDED(hr));

	hr = swapChain_->GetBuffer(1, IID_PPV_ARGS(&swapChainResources_[1]));
	assert(SUCCEEDED(hr));

	// RTV設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	// 先頭handle
	rtvHandles_[0] = rtvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	device_->CreateRenderTargetView(swapChainResources_[0], &rtvDesc, rtvHandles_[0]);
	// 2つ目のディスクリプタハンドルを自力で得る
	rtvHandles_[1].ptr = rtvHandles_[0].ptr + device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	// 2つ目の作成
	device_->CreateRenderTargetView(swapChainResources_[1], &rtvDesc, rtvHandles_[1]);
}

// 5.Fenceの生成とEventの作成
void DirectXCommon::CreateFence() {
	HRESULT hr;

	hr = device_->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(hr));

	// FenceのSignalを待つためのEventを作成
	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);
}

// 描画前の処理
void DirectXCommon::PreDraw() {
	// これから書き込むバッファのインデックスを取得
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();

	// TransitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier{};
	// バリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// バリアを張る対象のリソース。現在のバックバッファに対して行う
	barrier.Transition.pResource = swapChainResources_[backBufferIndex];
	// 遷移前(現在)のResourceState
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	// 遷移後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	// TransitionBarrierを張る
	commandList_->ResourceBarrier(1, &barrier);

	// 描画先のRTVを設定する
	commandList_->OMSetRenderTargets(1, &rtvHandles_[backBufferIndex], false, nullptr);
	// 指定した色で画面全体をクリアする
	float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };	// 青っぽい色
	commandList_->ClearRenderTargetView(rtvHandles_[backBufferIndex], clearColor, 0, nullptr);
}

// 描画後の処理
void DirectXCommon::PostDraw() {
	HRESULT hr;

	// 現在のBackBuffer取得
	UINT backBufferIndex =swapChain_->GetCurrentBackBufferIndex();

	// RenderTarget → Present
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource =swapChainResources_[backBufferIndex];
	barrier.Transition.StateBefore =D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter =D3D12_RESOURCE_STATE_PRESENT;

	commandList_->ResourceBarrier(1, &barrier);

	// コマンド確定
	hr = commandList_->Close();
	assert(SUCCEEDED(hr));

	// 実行
	ID3D12CommandList* commandLists[] = { commandList_ };
	commandQueue_->ExecuteCommandLists(1, commandLists);

	// 画面反映
	swapChain_->Present(1, 0);

	// GPU同期
	fenceValue_++;

	commandQueue_->Signal(fence_, fenceValue_);

	if (fence_->GetCompletedValue() < fenceValue_) {fence_->SetEventOnCompletion(fenceValue_,fenceEvent_);
		WaitForSingleObject(fenceEvent_, INFINITE);
	}

	// 次フレーム準備
	hr = commandAllocator_->Reset();
	assert(SUCCEEDED(hr));

	hr = commandList_->Reset(commandAllocator_,nullptr);
	assert(SUCCEEDED(hr));
}
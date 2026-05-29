#include "DirectXCommon.h"
#include "WinApp.h"
#include "Utility.h"
#include "Logger.h"
#include <format>			// フォーマットの自動指定をしてくれる(+安全策)
#include <cassert>
// Debug用のあれこれを使える様にする
#include <strsafe.h>
// ReportLiveObjects(DXGIに問い合わせできる様にする)
#include <dxgidebug.h>

// libのリンク
#pragma comment(lib,"dxguid.lib")

// デストラクタ
DirectXCommon::~DirectXCommon() {
	if (commandList_) commandList_->Release();
	if (commandAllocator_) commandAllocator_->Release();
	if (commandQueue_) commandQueue_->Release();
	for (uint32_t i = 0; i < swapChainDesc_.BufferCount; ++i) {
		if (swapChainResources_[i]) {
			swapChainResources_[i]->Release();
			swapChainResources_[i] = nullptr;
		}
	}
	if (swapChain_) swapChain_->Release();
	if (dsvDescriptorHeap_) dsvDescriptorHeap_->Release();
	if (depthStencilResource_) depthStencilResource_->Release();
	if (dsvDescriptorHeap_) {
		dsvDescriptorHeap_->Release();
		dsvDescriptorHeap_ = nullptr;
	}
	if (rtvDescriptorHeap_) rtvDescriptorHeap_->Release();
	if (srvDescriptorHeap_) srvDescriptorHeap_->Release();
	if (fence_) fence_->Release();
	if (fenceEvent_) CloseHandle(fenceEvent_);
	if (device_) device_->Release();
	if (dxgiFactory_) dxgiFactory_->Release();

#ifdef _DEBUG
	IDXGIDebug1* debug = nullptr;
	// DXGIのDebugインターフェースを取得できたら、リークしているオブジェクトがないか確認する
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		debug->Release();
	}

	if (debugController_) {
		debugController_->Release();
		debugController_ = nullptr;
	}
#endif
}

/*-------------------- DirectXの初期化 --------------------*/
void DirectXCommon::Initialize(HWND hwnd, int32_t width, int32_t height) {
	InitializeDebugLayer();		// デバッグレイヤーの初期化
	CreateDevice();				// 1. Factoryの生成、アダプタ選択、D3D12デバイス生成
	InitializeInfoQueue();		// Debug情報の初期化
	CreateCommand();			// 2. コマンドキュー、コマンドアロケータ、コマンドリストの生成
	CreateSwapChain(hwnd);		// 3. SwapChainの生成
	CreateRenderTargetView();	// 4. レンダーターゲットビューの生成
	// 深度バッファの生成
	depthStencilResource_ = CreateDepthStencilTextureResource(device_, width, height);	// DepthStencilTextureResourceを作成
	DepthStencilView();			// DepthStencilViewを作成
	CreateFence();				// 5. Fenceの生成
	CreateViewport();			// 描画用のViewPortとScissorRectの生成

	// descriptorSizeを取得
	DescriptorSizeSRV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	DescriptorSizeRTV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	DescriptorSizeDSV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

/*-------------------- デバッグレイヤーの初期化 --------------------*/
void DirectXCommon::InitializeDebugLayer() {
#ifdef _DEBUG
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController_)))) {
		// デバッグレイヤーを有効にする
		debugController_->EnableDebugLayer();
		// さらにGPU側でもチェックするようにする
		debugController_->SetEnableGPUBasedValidation(TRUE);
	}
#endif
}

/*-------------------- 1.Factoryの生成、アダプタ選択、D3D12デバイス生成 --------------------*/
void DirectXCommon::CreateDevice() {
	// HRESULT型はWindows系のエラーコードで、成功したか失敗したかを表す。SUCCEEDEDマクロで成功したかどうかを判定できる。
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
	// 初期化の根本部分でエラーが発生したら、プログラムが間違ってるかどうにもできない場合があるのでassertにしておく
	assert(SUCCEEDED(hr));

	// 使用するアダプタ用の変数(最初はnullptrで初期化)
	IDXGIAdapter4* useAdapter = nullptr;
	// 良い順にアダプタを頼む
	for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) !=
		DXGI_ERROR_NOT_FOUND; ++i) {
		// アダプタの情報を取得
		DXGI_ADAPTER_DESC3 adapterDesc{};
		useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));	// 取得できないのは一大事
		// ソフトウェアアダプタでなければ採用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			Log(std::format(L"Use Adapter: {}\n", adapterDesc.Description)
			);
			break;
		}
		useAdapter = nullptr;		// ソフトウェアアダプタだったので、使わない
	}
	// 良いアダプタが見つからなかったら、エラー
	assert(useAdapter != nullptr);

	// 機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelString[] = { "12.2","12.1","12.0" };
	// 高い順に生成できるか試す
	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		// 採用したアダプターデバイスを生成
		hr = D3D12CreateDevice(useAdapter, featureLevels[i], IID_PPV_ARGS(&device_));
		// 指定した機能レベルで生成できたか確認	
		if (SUCCEEDED(hr)) {
			// 生成できたのでログ出力を行ってループを抜ける
			Log(std::format("Feature Level {}\n", featureLevelString[i]));
			break;
		}
	}

	// アダプタを解放
	useAdapter->Release();

	// デバイスが生成できなかったらエラー
	assert(device_ != nullptr);
	Log("Complete create D3D12 device!\n");	// 初期化完了のログを出す
}

/*-------------------- Debug情報の初期化 --------------------*/
void DirectXCommon::InitializeInfoQueue() {
#ifdef _DEBUG
	ID3D12InfoQueue* infoQueue = nullptr;

	if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		// 重大なエラーが出たときに止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		// エラーが出たときに止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		// 警告時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);	// 解放忘れが判明した場合、一時的にコメントアウトして詳細を調べる

		// Windowsの不具合によるエラー、警告を無視する設定(現在は特に設定なし)

		// 解放
		infoQueue->Release();
	}
#endif
}

/*-------------------- 2.コマンドキュー、コマンドアロケータ、コマンドリストの生成 --------------------*/
void DirectXCommon::CreateCommand() {
	HRESULT hr;
	D3D12_COMMAND_QUEUE_DESC queueDesc{};
	hr = device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue_));
	// コマンドキューが生成できなかったらエラー
	assert(SUCCEEDED(hr));

	hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
	// コマンドアロケータが生成できなかったらエラー
	assert(SUCCEEDED(hr));

	hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_, nullptr, IID_PPV_ARGS(&commandList_));
	// コマンドリストが生成できなかったらエラー
	assert(SUCCEEDED(hr));
}

/*-------------------- 3.スワップチェーンの生成 --------------------*/
void DirectXCommon::CreateSwapChain(HWND hwnd) {
	HRESULT hr;
	swapChainDesc_.Width = WinApp::kClientWidth_;					// ウィンドウの横幅。クライアント領域と同じにする
	swapChainDesc_.Height = WinApp::kClientHeight_;					// ウィンドウの縦幅。クライアント領域と同じにする
	swapChainDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;				// 色の形式
	swapChainDesc_.SampleDesc.Count = 1;							// マルチサンプルしない
	swapChainDesc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// 描画のターゲットとして使う
	swapChainDesc_.BufferCount = 2;									// ダブルバッファ
	swapChainDesc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;		// モニタに映したら中身を破棄
	// コマンドキュー、ウィンドウハンドル、設定をしてスワップチェーンを生成
	hr = dxgiFactory_->CreateSwapChainForHwnd(commandQueue_, hwnd, &swapChainDesc_, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(&swapChain_));
	assert(SUCCEEDED(hr));	// スワップチェーンが生成できなかったらエラー	
}

/*-------------------- 4.RTV(レンダーターゲットビュー)の生成 --------------------*/
void DirectXCommon::CreateRenderTargetView() {
	HRESULT hr;

	// ディスクリプタヒープを作成(deviceは,Typeはレンダーターゲットビュー用,ダブルバッファ用に2つ,shaderVisibleはfalse)
	rtvDescriptorHeap_ = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	// SRV用の物も作成(deviceは,Typeはレンダーターゲットビュー用,数は128,shaderVisibleはtrue)
	srvDescriptorHeap_ = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

	// SwapChainからBackBuffer取得
	hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(&swapChainResources_[0]));
	// うまく所得できなければ起動できない
	assert(SUCCEEDED(hr));
	hr = swapChain_->GetBuffer(1, IID_PPV_ARGS(&swapChainResources_[1]));
	assert(SUCCEEDED(hr));

	// RTV設定
	rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;		// 出力結果をSRGBに変換して書き込む
	rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;	// 2Dテクスチャとして描画
	// ディスクリプタの先頭を取得
	rtvStartHandle_ = rtvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	// 1つ目の作成。1つ目は最初の所に、作る位置を指定してあげる
	rtvHandles_[0] = rtvStartHandle_;
	device_->CreateRenderTargetView(swapChainResources_[0], &rtvDesc_, rtvHandles_[0]);
	// 2つ目のディスクリプタハンドルを自力で得る
	rtvHandles_[1].ptr = rtvHandles_[0].ptr + device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	// 2つ目の作成
	device_->CreateRenderTargetView(swapChainResources_[1], &rtvDesc_, rtvHandles_[1]);
}

/*-------------------- 深度バッファの生成 --------------------*/
// DepthStencilTextureResourceを作成
ID3D12Resource* DirectXCommon::CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height) {
	// Resource設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	// Heap設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	// 深度初期値
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;
	//depthClearValue.DepthStencil.Stencil = 0;
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// Resource生成
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

// DepthStencilViewを作成
void DirectXCommon::DepthStencilView() {
	// DSV用のヒープでディスクリプタ数は1。DSVはShader内で触らないのでfalse
	dsvDescriptorHeap_ = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
	// DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;			// Format基本はResourceに合わせる
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;	// 2dTexture
	// DSVHeapの先頭にDSVを作る
	dsvHandleCPU_ = dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();	// 先に保存しておく(使い回しできる様に)
	device_->CreateDepthStencilView(depthStencilResource_, &dsvDesc, dsvHandleCPU_);
}

/*-------------------- 5.Fenceの生成とEventの作成 --------------------*/
void DirectXCommon::CreateFence() {
	// Fenceの生成。初期値は0、フラグはNone
	HRESULT hr = device_->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(hr));

	// FenceのSignalを待つためのEventを作成
	fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent_ != nullptr);
}

/*-------------------- 6.描画前の処理 --------------------*/
void DirectXCommon::PreDraw() {
	// これから書き込むバッファのインデックスを取得
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();

	// TransitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier{};
	// バリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// TransitionBarrierの設定。全てのサブリソースに対して行う
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	// バリアを張る対象のリソース。現在のバックバッファに対して行う
	barrier.Transition.pResource = swapChainResources_[backBufferIndex];
	// 遷移前(現在)のResourceState
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	// 遷移後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	// TransitionBarrierを張る
	commandList_->ResourceBarrier(1, &barrier);

	// 描画先のRTVとDSVを設定する
	commandList_->OMSetRenderTargets(1, &rtvHandles_[backBufferIndex], false, &dsvHandleCPU_);

	// ビューポートとシザー矩形を設定する
	commandList_->RSSetViewports(1, &viewPort_);
	commandList_->RSSetScissorRects(1, &scissorRect_);

	// 指定した色で画面全体をクリアする
	float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };	// 青っぽい色
	commandList_->ClearRenderTargetView(rtvHandles_[backBufferIndex], clearColor, 0, nullptr);
	commandList_->ClearDepthStencilView(dsvHandleCPU_, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// 描画用のDescriptorHeapの設定
	ID3D12DescriptorHeap* descripterHeaps[] = { srvDescriptorHeap_ };
	commandList_->SetDescriptorHeaps(1, descripterHeaps);
}

/*-------------------- 7.描画後の処理 --------------------*/
void DirectXCommon::PostDraw() {
	HRESULT hr;

	// 現在のBackBuffer取得
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();

	// 画面に描く処理が終わり、画面に映すので状態を遷移させる
	// RenderTargetからPresentにする
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = swapChainResources_[backBufferIndex];
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	// TransitionBarrierを張る
	commandList_->ResourceBarrier(1, &barrier);

	// コマンドリストの内容を確定させる。全てのコマンドを積んでからcloseする
	hr = commandList_->Close();
	assert(SUCCEEDED(hr));

	// GPUにコマンドリスト実行を指示する
	ID3D12CommandList* commandLists[] = { commandList_ };
	commandQueue_->ExecuteCommandLists(1, commandLists);
	// GPUとOSに画面の交換を指示する(ここで画面反映)
	swapChain_->Present(1, 0);

	// Fenceの値の更新
	fenceValue_++;
	// GPUがここまでたどり着いた時、Fenceの値を指定した値に代入するようにSignalを送る
	commandQueue_->Signal(fence_, fenceValue_);
	// Fenceの値が指定したSignalの値にたどり着いているか確認する
	// GetCompletedValueの初期値はFence作成時に渡した初期値
	if (fence_->GetCompletedValue() < fenceValue_) {
		// まだたどり着いていないので、Signalの値にたどり着くまで待つ様にイベントを設定する
		fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
		// イベントが起きるまで待つ
		WaitForSingleObject(fenceEvent_, INFINITE);
	}

	// 次のフレーム用のコマンドリストを準備する
	hr = commandAllocator_->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList_->Reset(commandAllocator_, nullptr);
	assert(SUCCEEDED(hr));
}

/*-------------------- コマンドを実行 --------------------*/
void DirectXCommon::ExecuteCommandList() {
	// コマンドリストを閉じる
	HRESULT hr = commandList_->Close();
	assert(SUCCEEDED(hr));
	// GPUへ投げる
	ID3D12CommandList* commandLists[] = { commandList_ };
	commandQueue_->ExecuteCommandLists(1, commandLists);
}

/*-------------------- GPUを待機 --------------------*/
void DirectXCommon::WaitForGPU() {
	fenceValue_++;
	commandQueue_->Signal(fence_, fenceValue_);
	if (fence_->GetCompletedValue() < fenceValue_) {
		fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
		WaitForSingleObject(fenceEvent_, INFINITE);
	}
}

/*-------------------- commandListのReset --------------------*/
void DirectXCommon::ResetCommandList() {
	commandAllocator_->Reset();
	commandList_->Reset(commandAllocator_, nullptr);
}

/*-------------------- ViewPortとScissorRectの生成 --------------------*/
void DirectXCommon::CreateViewport() {
	// ビューポートの設定
	viewPort_.Width = static_cast<float>(WinApp::kClientWidth_);
	viewPort_.Height = static_cast<float>(WinApp::kClientHeight_);
	viewPort_.MinDepth = 0.0f;
	viewPort_.MaxDepth = 1.0f;
	// シザー矩形の設定
	scissorRect_.left = 0;
	scissorRect_.top = 0;
	scissorRect_.right = WinApp::kClientWidth_;
	scissorRect_.bottom = WinApp::kClientHeight_;
}
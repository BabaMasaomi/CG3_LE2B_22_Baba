#include "DirectXCommon.h"
#include "WinApp.h"
#include "Logger.h"
#include <format>			// フォーマットの自動指定をしてくれる(+安全策)
#include <cassert>
// Debug用のあれこれを使える様にする
#include <dbghelp.h>
#include <strsafe.h>
// ReportLiveObjects(DXGIに問い合わせできる様にする)
#include <dxgidebug.h>

// libのリンク
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib,"dxguid.lib")

// デストラクタ
DirectXCommon::~DirectXCommon() {
	CloseHandle(fenceEvent_);
	fence_->Release();
	rtvDescriptorHeap_->Release();
	swapChainResources_[0]->Release();
	swapChainResources_[1]->Release();
	swapChain_->Release();
	commandList_->Release();
	commandAllocator_->Release();
	commandQueue_->Release();
	device_->Release();
	dxgiFactory_->Release();

#ifdef _DEBUG
	IDXGIDebug1* debug = nullptr;

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
void DirectXCommon::Initialize(HWND hwnd) {
	InitializeDebugLayer();		// デバッグレイヤーの初期化
	CreateDevice();				// 1. Factoryの生成、アダプタ選択、D3D12デバイス生成
	InitializeInfoQueue();		// Debug情報の初期化
	CreateCommand();			// 2. コマンドキュー、コマンドアロケータ、コマンドリストの生成
	CreateSwapChain(hwnd);		// 3. SwapChainの生成
	CreateRenderTargetView();	// 4. レンダーターゲットビューの生成
	CreateFence();				// 5. Fenceの生成
}

/*-------------------- デバッグレイヤーの初期化 --------------------*/
void DirectXCommon::InitializeDebugLayer() {
#ifdef _DEBUG
	if (SUCCEEDED(
		D3D12GetDebugInterface(
			IID_PPV_ARGS(&debugController_)))) {

		debugController_->EnableDebugLayer();
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
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = WinApp::kClientWidth;						// ウィンドウの横幅。クライアント領域と同じにする
	swapChainDesc.Height = WinApp::kClientHeight;					// ウィンドウの縦幅。クライアント領域と同じにする
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;				// 色の形式
	swapChainDesc.SampleDesc.Count = 1;								// マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// 描画のターゲットとして使う
	swapChainDesc.BufferCount = 2;									// ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;		// モニタに映したら中身を破棄
	// コマンドキュー、ウィンドウハンドル、設定をしてスワップチェーンを生成
	hr = dxgiFactory_->CreateSwapChainForHwnd(commandQueue_, hwnd, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(&swapChain_));
	assert(SUCCEEDED(hr));	// スワップチェーンが生成できなかったらエラー	
}

/*-------------------- 4.RTV(レンダーターゲットビュー)の生成 --------------------*/
void DirectXCommon::CreateRenderTargetView() {
	HRESULT hr;

	// ディスクリプタヒープを作成
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;	// レンダーターゲットビュー用
	rtvDescriptorHeapDesc.NumDescriptors = 2;						// ダブルバッファ用に2つ
	hr = device_->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap_));
	// ディスクリプタヒープが生成できなかったらエラー
	assert(SUCCEEDED(hr));

	// SwapChainからBackBuffer取得
	hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(&swapChainResources_[0]));
	// うまく所得できなければ起動できない
	assert(SUCCEEDED(hr));
	hr = swapChain_->GetBuffer(1, IID_PPV_ARGS(&swapChainResources_[1]));
	assert(SUCCEEDED(hr));

	// RTV設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;				// 出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;			// 2Dテクスチャとして描画
	// ディスクリプタの先頭を取得
	rtvStartHandle_ = rtvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	// 1つ目の作成。1つ目は最初の所に、作る位置を指定してあげる
	rtvHandles_[0] = rtvStartHandle_;
	device_->CreateRenderTargetView(swapChainResources_[0], &rtvDesc, rtvHandles_[0]);
	// 2つ目のディスクリプタハンドルを自力で得る
	rtvHandles_[1].ptr = rtvHandles_[0].ptr + device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	// 2つ目の作成
	device_->CreateRenderTargetView(swapChainResources_[1], &rtvDesc, rtvHandles_[1]);
}

/*-------------------- 5.Fenceの生成とEventの作成 --------------------*/
void DirectXCommon::CreateFence() {
	HRESULT hr;

	hr = device_->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
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
	// 
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
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
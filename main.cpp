#include <Windows.h>
#include "logger.h"
#include <format>			// フォーマットの自動指定をしてくれる(+安全策)
#include "WinApp.h"
#include "DirectXCommon.h"
#include "DXCManager.h"
#include "Input.h"
#include "TextureManager.h"
#include "Camera.h"
#include "DirectionalLight.h"
#include "Sprite.h"
#include "Object3d.h"
#include "ModelPSO.h"
#include "Sound.h"
// Debug用のあれこれを使える様にする
#include <dbghelp.h>
#include <strsafe.h>
#include <dxgidebug.h>
// ImGui
#ifdef USE_IMGUI
#include "ImGuiManager.h"
#endif

// libのリンク
#pragma comment(lib, "Dbghelp.lib")

struct D3D12ResourceLeakChecker {
	~D3D12ResourceLeakChecker() {
		// リソースリークチェック
		Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		}
	}
};

/*-------------------- 関数 --------------------*/
// クラッシュしたときにDumpファイルを出力する関数
static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exceptionPointers) {
	// 時刻を取得して、時刻を名前に入れたファイルを作成。Dumpsディレクトリ以下に保存する
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = { 0 };
	CreateDirectory(L"./Dumps", nullptr);
	StringCchPrintfW(filePath, MAX_PATH, L"./Dumps/%04d-%02d%02d-%02d%02d.dmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);
	HANDLE dumpFileHandle = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
	// processID(このexeのID)とクラッシュ(例外)の発生したthreadIDを取得
	DWORD processID = GetCurrentProcessId();
	DWORD threadID = GetCurrentThreadId();
	// 設定情報を入力
	MINIDUMP_EXCEPTION_INFORMATION minidumpinfomation{ 0 };
	minidumpinfomation.ThreadId = threadID;
	minidumpinfomation.ExceptionPointers = exceptionPointers;
	minidumpinfomation.ClientPointers = TRUE;
	// Dumpを出力。MiniDumpNormalは最低限の情報を出力するフラグ
	MiniDumpWriteDump(GetCurrentProcess(), processID, dumpFileHandle, MiniDumpNormal, &minidumpinfomation, nullptr, nullptr);
	// 他に関連づけられているSEH例外ハンドラがあれば実行。通常プロセスは終了する
	return EXCEPTION_EXECUTE_HANDLER;
}


// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	// COM初期化
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	// WinMainの頭に呼んで、正しくリークチェックする
	D3D12ResourceLeakChecker leakCheck;

	// 誰も捕捉しなかった場合に、捕捉する関数を登録
	SetUnhandledExceptionFilter(ExportDump);

	// Logファイルの準備
	std::ofstream logStream = CreateLogFile();

	// ゲームウィンドウの表示処理
	WinApp winApp;
	winApp.Initialize();

	// DirectXの初期化
	DirectXCommon dxCommon;
	dxCommon.Initialize(winApp.GetHwnd(), winApp.kClientWidth_, winApp.kClientHeight_);

	// DXCの初期化
	DXCManager dxcManager;
	dxcManager.Initialize();

	// InPutの初期化
	Input input;
	input.Initialize(hInstance, winApp.GetHwnd());

	// TextureManagerの初期化
	TextureManager textureManager;
	textureManager.Initialize(&dxCommon);

	dxCommon.ExecuteCommandList();	// コマンドを実行
	dxCommon.WaitForGPU();			// GPUを待機
	dxCommon.ResetCommandList();	// commandListのReset

	// ImGuiの初期化
#ifdef USE_IMGUI
	ImGuiManager imguiManager;
	imguiManager.Initialize(&winApp, &dxCommon);
#endif

	// カメラの初期化
	Camera camera;
	camera.Initialize(WinApp::kClientWidth_, WinApp::kClientHeight_, &input);

	// 平行光源の初期化
	DirectionalLight directionalLight;
	directionalLight.Initialize(&dxCommon);

	// ModelPSOの初期化
	ModelPSO modelPSO;
	modelPSO.Initialize(&dxCommon, &dxcManager);

	// Object3dの初期化
	Object3d object3d;
	object3d.Initialize(&dxCommon, &modelPSO, &textureManager, &directionalLight);

	// Spriteの初期化
	Sprite sprite;
	sprite.Initialize(&dxCommon, &textureManager, &winApp);

	// Soundの初期化
	Sound sound;
	sound.Initialize();
	// 音声読み込み
	SoundData soundData1 = sound.SoundLoadWave("resources/fanfare.wav");
	// サウンド再生
	sound.SoundPlayeWave(soundData1);

	// ログ出力
	Log(logStream, "Start logging!");

	/*-------------------- 変数 --------------------*/
	// ImGui用に変数を用意
	bool showSprite = false;

	/*-------------------- ゲームループの処理 --------------------*/
	MSG msg{};
	// ウィンドウのxボタンが押されるまでループ
	while (!winApp.IsEndGame()) {
		winApp.ProcessMessage();

		/*-------------------- 更新処理 --------------------*/
		input.Update();
		camera.Update();
		directionalLight.Update();
		object3d.Update(camera);
		sprite.Update();

		if (input.TriggerKey(DIK_F1)) {
			camera.SetDebugMode(!camera.GetDebugMode());
			char buffer[128];
			sprintf_s(buffer, "current mode: %d\n", camera.GetDebugMode());
			OutputDebugStringA(buffer);
		}

		/*-------------------- 描画処理 --------------------*/
		dxCommon.PreDraw();		// 描画前の処理

		// ImGui
#ifdef USE_IMGUI
		imguiManager.Begin();	// 以下にImGuiの内容を記述
		// Lightを調整
		if (ImGui::CollapsingHeader("Light")) {
			ImGui::ColorEdit4("Color", &directionalLight.GetColor().x);							// 色を変更
			ImGui::DragFloat3("Direction", &directionalLight.GetDirection().x, 0.01f);				// 向きを変更
			ImGui::DragFloat("Intensity", &directionalLight.GetIntensity(), 0.01f, 0.0f, 10.0f);	// 輝度を変更
		}
		// Modelを調整
		if (ImGui::CollapsingHeader("Model")) {
			ImGui::PushID(1);
			ImGui::DragFloat3("Scale", &object3d.GetScale().x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat3("Rotate", &object3d.GetRotate().x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat3("Translate", &object3d.GetTranslate().x, 0.01f);
			ImGui::PopID();
		}
		// SpriteのUVTransformを調整
		if (ImGui::CollapsingHeader("UVTransform")) {
			ImGui::PushID(2);
			ImGui::Checkbox("Sprite", &showSprite);
			ImGui::DragFloat2("Scale", &sprite.GetUVTransformScale().x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat2("Translate", &sprite.GetUVTransformTranslate().x, 0.01f, -10.0f, 10.0f);
			ImGui::SliderAngle("Rotate", &sprite.GetUVTransformRotate().z);
			ImGui::PopID();
		}

		imguiManager.End();		// 以上でImGuiの内容を終える
#endif

		// モデルの描画
		object3d.Draw();

		// スプライトの描画
		if (showSprite) {
			sprite.Draw();
		}

		// ImGuiの描画
#ifdef USE_IMGUI
		imguiManager.Draw();
#endif

		dxCommon.PostDraw();	// 描画後の処理

	}

	// ImGuiの終了処理
#ifdef USE_IMGUI
	imguiManager.Finalize();
#endif

	// 音声データ解放
	sound.SoundUnload(&soundData1);
	sound.Finalize();

	dxCommon.WaitForGPU();

	// COMの終了処理
	CoUninitialize();

	return 0;
}
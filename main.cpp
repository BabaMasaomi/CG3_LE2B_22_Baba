#include <Windows.h>
#include "logger.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "DXCManager.h"
#include "TextureManager.h"
#include "Camera.h"
#include "Sprite.h"
#include "Sphere.h"
#include "SpherePSO.h"
// Debug用のあれこれを使える様にする
#include <dbghelp.h>
#include <strsafe.h>
// ImGui
#ifdef USE_IMGUI
#include "ImGuiManager.h"
#endif

// libのリンク
#pragma comment(lib, "Dbghelp.lib")

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
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	// COM初期化
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

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

	// TextureManagerの初期化
	TextureManager textureManager;
	textureManager.Initialize(&dxCommon);
	// テクスチャの読み込み
	//ID3D12Resource* intermediateResource = textureManager.LoadTexture("resources/uvChecker.png");
	textureManager.LoadTexture("resources/uvChecker.png", 1);
	textureManager.LoadTexture("resources/monsterBall.png", 2);

	dxCommon.ExecuteCommandList();	// コマンドを実行
	dxCommon.WaitForGPU();			// GPUを待機
	dxCommon.ResetCommandList();	// commandListのReset
	//// intermediateResourceをRelease
	//if (intermediateResource) {
	//	intermediateResource->Release();
	//	intermediateResource = nullptr;
	//}

	// ImGuiの初期化
#ifdef USE_IMGUI
	ImGuiManager imguiManager;
	imguiManager.Initialize(&winApp, &dxCommon);
#endif

	// カメラの初期化
	Camera camera;
	camera.Initialize(WinApp::kClientWidth_, WinApp::kClientHeight_);

	// SpherePSOの初期化
	SpherePSO spherePSO;
	spherePSO.Initialize(&dxCommon, &dxcManager);

	// Sphereの初期化
	Sphere sphere;
	sphere.Initialize(&dxCommon, &spherePSO, &textureManager);


	// Spriteの初期化
	Sprite sprite;
	sprite.Initialize(&dxCommon,&textureManager, &winApp);

	// ログ出力
	Log(logStream, "Start logging!");

	/*-------------------- ゲームループの処理 --------------------*/
	MSG msg{};
	// ウィンドウのxボタンが押されるまでループ
	while (!winApp.IsEndGame()) {
		winApp.ProcessMessage();

		/*-------------------- 更新処理 --------------------*/
		camera.Update();
		sphere.Update(camera);
		sprite.Update();

		/*-------------------- 描画処理 --------------------*/
		dxCommon.PreDraw();		// 描画前の処理

		// ImGui
#ifdef USE_IMGUI
		imguiManager.Begin();	// 以下にImGuiの内容を記述
		// デモウィンドウ表示
		ImGui::ShowDemoWindow();
		imguiManager.End();		// 以上でImGuiの内容を終える
#endif

		// 三角形の描画
		//triangle.Draw();
		sphere.Draw(2);

		// スプライトの描画
		sprite.Draw(1);

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
	// COMの終了処理
	CoUninitialize();
	return 0;
}
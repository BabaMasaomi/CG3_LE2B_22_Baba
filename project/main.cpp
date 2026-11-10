#include <Windows.h>
#include "logger.h"
#include <format>			// フォーマットの自動指定をしてくれる(+安全策)
#include "WinApp.h"
#include "DirectXCommon.h"
#include "DXCManager.h"
#include "Input.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "Camera.h"
#include "DirectionalLight.h"
#include "SpritePSO.h"
#include "ModelPSO.h"
#include "Model.h"
#include "Object3D.h"
// オブジェクト描画用
#include <memory>
#include <vector>
#include <string>
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

using namespace Engine;

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
	dxCommon.Initialize(&winApp);

	// DXCの初期化
	DXCManager dxcManager;
	dxcManager.Initialize();

	// InPutの初期化
	Input input;
	input.Initialize(hInstance, winApp.GetHwnd());

	// TextureManagerの初期化
	TextureManager textureManager;
	textureManager.Initialize(&dxCommon);

	// ModelManagerの初期化
	ModelManager modelManager;
	modelManager.Initialize(&dxCommon, &textureManager);

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
	camera.Initialize(winApp.GetClientWidth(), winApp.GetClientHeight(), &input);

	// 平行光源の初期化
	DirectionalLight directionalLight;
	directionalLight.Initialize(&dxCommon);
	/*-------------------- モデルの初期化 --------------------*/

	// SpritePSOの初期化
	SpritePSO spritePSO;
	spritePSO.Initialize(&dxCommon, &dxcManager);

	// ModelPSOの初期化
	ModelPSO modelPSO;
	modelPSO.Initialize(&dxCommon, &dxcManager);

	// 板ポリのモデルを読み込む
	Model* planeModel = modelManager.LoadModel("plane.obj");

	// 板ポリ10枚をまとめて描画するObject3D
	Object3D plane;
	plane.Initialize(&dxCommon, &modelPSO, &directionalLight);
	plane.SetModel(planeModel);
	plane.SetInstanceCount(10);

	// 1枚ずつ左上へずらして配置
	for (uint32_t i = 0; i < 10; ++i) {
		const float offset = static_cast<float>(i);

		Transform instance{
			{1.0f, 1.0f, 1.0f},
			{0.0f, 0.0f, 0.0f},
			{-0.1f * offset, 0.1f * offset, -0.1f * offset}
		};

		plane.SetInstanceTransform(i, instance);
	}

	// ログ出力
	Log(logStream, "Start logging!");

	/*-------------------- 変数 --------------------*/
	// Comboで現在選択している音声
	int selectedSoundIndex = 0;

	/*-------------------- ゲームループの処理 --------------------*/
	MSG msg{};
	// ウィンドウのxボタンが押されるまでループ
	while (!winApp.IsEndGame()) {
		winApp.ProcessMessage();

		/*-------------------- 更新処理 --------------------*/
		// プロジェクトの種類問わず3種は固定配置
		input.Update();
		camera.Update();
		directionalLight.Update();

		// 板ポリを配置
		plane.Update(camera);

		/*-------------------- 描画処理 --------------------*/
		dxCommon.PreDraw();		// 描画前の処理

		// ImGui
#ifdef USE_IMGUI
		imguiManager.Begin();	// 以下にImGuiの内容を記述
		ImGui::Text("DebugCamera");
		// デバッグカメラを起動
		bool debugMode = camera.GetDebugMode();
		if (ImGui::Checkbox("Enable", &debugMode)) {
			camera.SetDebugMode(debugMode);
		}
		// cameraの位置をリセット
		if (ImGui::Button("Reset")) {
			camera.ResetDebugCamera();
		}

		ImGui::Separator();
		// 板ポリ10枚を表示
		if (ImGui::CollapsingHeader(
			"Plane", ImGuiTreeNodeFlags_DefaultOpen)) {

			// 10枚全体の変換
			Vector3 scale = plane.GetScale();
			Vector3 rotate = plane.GetRotate();
			Vector3 translate = plane.GetTranslate();

			if (ImGui::DragFloat3(
				"Scale", &scale.x, 0.01f, 0.01f, 100.0f)) {
				plane.SetScale(scale);
			}

			if (ImGui::DragFloat3("Rotate", &rotate.x, 0.01f)) {
				plane.SetRotate(rotate);
			}

			if (ImGui::DragFloat3("Translate", &translate.x, 0.01f)) {
				plane.SetTranslate(translate);
			}

			// 10枚共通の色
			Vector4 color = plane.GetColor();

			if (ImGui::ColorEdit4(
				"Color", &color.x, ImGuiColorEditFlags_Float)) {
				plane.SetColor(color);
			}

			// 10枚共通のUV
			Transform uv = plane.GetUVTransform();
			bool uvChanged = false;

			uvChanged |= ImGui::DragFloat2(
				"UV Scale", &uv.scale.x, 0.01f, 0.01f, 10.0f);

			uvChanged |= ImGui::DragFloat2(
				"UV Translate", &uv.translate.x, 0.01f);

			uvChanged |= ImGui::DragFloat(
				"UV Rotate", &uv.rotate.z, 0.01f);

			if (uvChanged) {
				plane.SetUVTransform(uv);
			}
		}

		// ImGuiで変更した値を、このフレームの描画に反映
		plane.Update(camera);

		ImGui::Separator();
		// Lightを調整
		if (ImGui::CollapsingHeader("Light")) {
			Vector4 color = directionalLight.GetColor();
			Vector3 direction = directionalLight.GetDirection();
			float intensity = directionalLight.GetIntensity();
			if (ImGui::ColorEdit4("Color", &color.x/*, ImGuiColorEditFlags_Float*/)) {	// 色を変更
				directionalLight.SetColor(color);
			}
			if (ImGui::DragFloat3("Direction", &direction.x, 0.01f)) {					// 向きを変更
				directionalLight.SetDirection(direction);
			}
			if (ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.0f, 10.0f)) {		// 輝度を変更
				directionalLight.SetIntensity(intensity);
			}
		}

		imguiManager.End();		// 以上でImGuiの内容を終える
#endif

		// 板ポリ10枚をInstancing描画
		plane.Draw();

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

	dxCommon.WaitForGPU();

	// COMの終了処理
	CoUninitialize();

	return 0;
}
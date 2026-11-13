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
#include "ParticlePSO.h"
#include "Model.h"
#include "ParticleSystem.h"
// オブジェクト描画用
#include <memory>
#include <vector>
#include <string>
#include <chrono>
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

	/*-------------------- パーティクルの初期化 --------------------*/
	// ParticlePSOの初期化
	ParticlePSO particlePSO;
	particlePSO.Initialize(&dxCommon, &dxcManager);

	// 板ポリのモデルを読み込む
	Model* planeModel = modelManager.LoadModel("CG3Particle.obj");

	// 板ポリ10枚をまとめて描画するParticleのInstancing描画
	ParticleSystem particle;
	particle.Initialize(&dxCommon, &particlePSO, &directionalLight);
	particle.SetModel(planeModel);

	// 出現数
	int spawnCount = 20;

	// 出現したパーティクル
	auto emitParticles = [&]() {
		const Transform spawnTransform{
			{0.2f, 0.2f, 0.2f},
			{0.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 0.0f}
		};

		// ここでパーティクル生成
		particle.MakeNewParticle(static_cast<uint32_t>(spawnCount), spawnTransform);

		};

	// 起動時にも一度生成する
	emitParticles();

	// ゲームループ前の時刻を記録
	auto previousTime = std::chrono::steady_clock::now();

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

		// このループでの現在時刻を記録
		const auto now = std::chrono::steady_clock::now();
		// デルタタイムを取得
		const float deltaTime = std::chrono::duration<float>(now - previousTime).count();
		// 次回ループで直前の時間として使用
		previousTime = now;

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
		ImGui::Text("Particle : Emit&Clear");
		// Particleの生成を管理
		ImGui::SliderInt("Spawn Count", &spawnCount, 1, 200);

		if (ImGui::Button("Emit")) {
			emitParticles();
		}

		ImGui::SameLine();

		if (ImGui::Button("Clear")) {
			particle.ClearParticles();
		}

		ImGui::Text("Alive: %u", particle.GetParticleCount());

		ImGui::Separator();
		// パーティクルのBillboardかを変更
		bool billboardEnabled = particle.IsBillboardEnabled();
		// BillboardのON/OFF
		if (ImGui::Checkbox("Billboard", &billboardEnabled)) {
			particle.SetBillboardEnabled(billboardEnabled);
		}


		ImGui::Separator();
		// パーティクルのUV,Lightingを編集
		ImGui::Text("Particle UV&Lighting");

		// パーティクル全体の変換
		Vector3 scale = particle.GetScale();
		Vector3 rotate = particle.GetRotate();
		Vector3 translate = particle.GetTranslate();

		if (ImGui::DragFloat3(
			"Scale", &scale.x, 0.01f, 0.01f, 100.0f)) {
			particle.SetScale(scale);
		}

		if (ImGui::DragFloat3("Rotate", &rotate.x, 0.01f)) {
			particle.SetRotate(rotate);
		}

		if (ImGui::DragFloat3("Translate", &translate.x, 0.01f)) {
			particle.SetTranslate(translate);
		}

		// パーティクルの共通の色
		Vector4 color = particle.GetColor();

		if (ImGui::ColorEdit4(
			"UV Color", &color.x, ImGuiColorEditFlags_Float)) {
			particle.SetColor(color);
		}

		// パーティクルの共通のUV
		Transform uv = particle.GetUVTransform();
		bool uvChanged = false;

		uvChanged |= ImGui::DragFloat2("UV Scale", &uv.scale.x, 0.01f, 0.01f, 10.0f);
		uvChanged |= ImGui::DragFloat2("UV Translate", &uv.translate.x, 0.01f);
		uvChanged |= ImGui::DragFloat("UV Rotate", &uv.rotate.z, 0.01f);

		if (uvChanged) {
			particle.SetUVTransform(uv);
		}


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

		// パーティクルを更新、描画
		particle.Update(camera, deltaTime);
		particle.Draw();

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
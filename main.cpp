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
#include "Sprite.h"
#include "ModelPSO.h"
#include "Model.h"
#include "Object3D.h"
#include "Sphere.h"
#include "Audio.h"
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

	// SpritePSOの初期化
	SpritePSO spritePSO;
	spritePSO.Initialize(&dxCommon, &dxcManager);

	// Spriteの初期化(デバッグ用Spriteとして先頭で初期化し、uvCheckerを Index = 1 で固定しておく)
	Sprite sprite;
	sprite.Initialize(&dxCommon, &spritePSO, &textureManager, &winApp);

	/*-------------------- モデルの初期化 --------------------*/
	// ModelPSOの初期化
	ModelPSO modelPSO;
	modelPSO.Initialize(&dxCommon, &dxcManager);

	// モデルの複数描画の準備
	struct ModelEntry {
		std::string displayName;
		std::string directoryPath;
		std::string filename;
		Model* model = nullptr;
	};

	// SphereをModelとして生成
	Model* sphereModel = modelManager.CreateSphereModel("UnitSphere", 1.0f, 16, "resources/uvChecker.png");

	// モデルの読み込み
	std::vector<ModelEntry> modelEntries = {
	{"Sphere", "", "", sphereModel},		// SphereはOBJではないため、directoryPathとfilenameは空文字
	{"Axis",   "resources", "axis.obj",   nullptr},
	{"Plane",   "resources", "plane.obj",   nullptr},
	{"Bunny", "resources",  "bunny.obj", nullptr},
	{"Teapot", "resources",  "teapot.obj", nullptr},
	};

	// 各モデルの読み込み
	for (ModelEntry& entry : modelEntries) {
		// modelが未設定の項目だけOBJから読み込む
		if (entry.model == nullptr) {
			entry.model = modelManager.LoadModel(
				entry.directoryPath,
				entry.filename
			);
		}
	}

	// 配置済みObject3Dを管理
	std::vector<std::unique_ptr<Object3D>> objects;

	//// テクスチャの読み込み
	//textureManager.LoadTexture("resources/uvChecker.png");

	//// 球の初期化
	//Sphere sphere;
	//sphere.Initialize(&dxCommon, &modelPSO, &textureManager, &directionalLight);

	// Audioの初期化
	Audio audio;
	audio.Initialize();
	// 音声読み込み
	SoundData audioData1 = audio.AudioLoadWave("resources/fanfare.wav");
	// サウンド再生
	//audio.AudioPlayWave(audioData1);

	// ログ出力
	Log(logStream, "Start logging!");

	/*-------------------- 変数 --------------------*/
	// 選択したモデルを記録
	int selectedModelIndex = 0;
	// spriteを表示するか
	bool showSprite = false;

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

		// 作りたい物に合わせて配置
		// モデルの更新
		for (const auto& object : objects) {
			object->Update(camera);
		}
		//sphere.Update(camera);
		sprite.Update();

		/*-------------------- 描画処理 --------------------*/
		dxCommon.PreDraw();		// 描画前の処理

		// ImGui
#ifdef USE_IMGUI
		imguiManager.Begin();	// 以下にImGuiの内容を記述
		// cameraの位置をリセット
		if (ImGui::CollapsingHeader("DebugCamera")) {
			bool debugMode = camera.GetDebugMode();
			if (ImGui::Checkbox("Enable", &debugMode)) {
				camera.SetDebugMode(debugMode);
			}
			if (ImGui::Button("Reset")) {
				camera.Initialize(winApp.GetClientWidth(), winApp.GetClientHeight(), &input);
			}
		}

		ImGui::Separator();
		// Modelを追加
		if (ImGui::BeginCombo("Model Type", modelEntries[selectedModelIndex].displayName.c_str())) {
			// 追加するモデルを選択
			for (int i = 0; i < static_cast<int>(modelEntries.size()); ++i) {
				bool isSelected = (selectedModelIndex == i);
				// 選択肢を表示
				if (ImGui::Selectable(modelEntries[i].displayName.c_str(), isSelected)) {
					selectedModelIndex = i;
				}
				// 選んだ選択肢をデフォルトフォーカスにする
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		// モデルを追加する
		if (ImGui::Button("Add Model")) {
			auto object = std::make_unique<Object3D>();
			// モデルを初期化
			object->Initialize(&dxCommon, &modelPSO, &directionalLight);
			// モデルをセット
			object->SetModel(modelEntries[selectedModelIndex].model);
			// リストに追加
			objects.push_back(std::move(object));
		}

		// 削除するモデルを記録する変数
		int deleteObjectIndex = -1;

		// 配置済みモデルの調整
		for (size_t i = 0; i < objects.size(); ++i) {
			Object3D& object = *objects[i];
			ImGui::PushID(static_cast<int>(i));
			// ツリーノードのラベルを作成
			std::string label = "Objects " + std::to_string(i);
			// 個別でモデルを調整
			if (ImGui::CollapsingHeader(label.c_str())) {
				Vector3 scale = object.GetScale();
				Vector3 rotate = object.GetRotate();
				Vector3 translate = object.GetTranslate();
				// SRTを変更
				if (ImGui::DragFloat3("Scale", &scale.x, 0.01f, 0.01f, 100.0f)) {	// スケールを変更
					object.SetScale(scale);
				}
				if (ImGui::DragFloat3("Rotate", &rotate.x, 0.01f)) {				// 回転を変更
					object.SetRotate(rotate);
				}
				if (ImGui::DragFloat3("Translate", &translate.x, 0.01f)) {			// 移動を変更
					object.SetTranslate(translate);
				}
				// モデルを削除
				if (ImGui::Button("Delete")) {
					deleteObjectIndex = static_cast<int>(i);
				}
				// UV、色、Lightingを調整
				if (ImGui::TreeNode("Model")) {
					// ライティングモードを記録する
					int lightingMode = static_cast<int>(object.GetLightingMode());

					ImGui::Combo("Lighting", &lightingMode, "None\0Lambert\0Half Lambert\0");	// ライティングを変更
					object.SetLightingMode(static_cast<LightingMode>(lightingMode));

					ImGui::TreePop();
				}
			}
			ImGui::PopID();
		}
		// 削除するモデルがあれば削除
		if (deleteObjectIndex >= 0) {
			objects.erase(objects.begin() + deleteObjectIndex);
		}

		ImGui::Separator();
		// Lightを調整
		if (ImGui::CollapsingHeader("Light")) {
			Vector4 color = directionalLight.GetColor();
			Vector3 direction = directionalLight.GetDirection();
			float intensity = directionalLight.GetIntensity();
			if (ImGui::ColorEdit4("Color", &color.x)) {								// 色を変更
				directionalLight.SetColor(color);
			}
			if (ImGui::DragFloat3("Direction", &direction.x, 0.01f)) {				// 向きを変更
				directionalLight.SetDirection(direction);
			}
			if (ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.0f, 10.0f)) {	// 輝度を変更
				directionalLight.SetIntensity(intensity);
			}
			//ImGui::Combo("Lighting", &lightingMode, "None\0Lambert\0Half Lambert\0");	// ライティングを変更
			//directionalLight.SetLightingMode(static_cast<LightingMode>(lightingMode));
		}

		ImGui::Separator();
		// Spriteを調整
		if (ImGui::CollapsingHeader("Sprite")) {
			ImGui::Checkbox("Show", &showSprite);
			Vector3 UVScale = sprite.GetUVTransformScale();
			Vector3 UVTranslate = sprite.GetUVTransformTranslate();
			Vector3 UVRotate = sprite.GetUVTransformRotate();
			if (ImGui::DragFloat2("SpriteScale", &UVScale.x, 0.01f, -10.0f, 10.0f)) {			// スケールを変更
				sprite.SetUVTransformScale(UVScale);
			}
			if (ImGui::DragFloat2("SpriteTranslate", &UVTranslate.x, 0.01f, -10.0f, 10.0f)) {	// 回転を変更
				sprite.SetUVTransformTranslate(UVTranslate);
			}
			if (ImGui::SliderAngle("SpriteRotate", &UVRotate.z)) {								// 移動を変更
				sprite.SetUVTransformRotate(UVRotate);
			}
		}

		imguiManager.End();		// 以上でImGuiの内容を終える
#endif

		// モデルの描画
		for (const auto& object : objects) {
			object->Draw();
		}

		// 球の描画
		//sphere.Draw(1);

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
	audio.Finalize();

	dxCommon.WaitForGPU();

	// COMの終了処理
	CoUninitialize();

	return 0;
}
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
	/*-------------------- モデルの初期化 --------------------*/

	// SpritePSOの初期化
	SpritePSO spritePSO;
	spritePSO.Initialize(&dxCommon, &dxcManager);

	// ModelPSOの初期化
	ModelPSO modelPSO;
	modelPSO.Initialize(&dxCommon, &dxcManager);

	// レンダーの種類
	enum class RenderObjectType {
		kModel3D,
		kSprite2D,
	};

	// モデルの複数描画の構造体
	struct RenderEntry {
		std::string displayName;
		RenderObjectType  type;

		// 3Dモデル用
		std::string filename;
		Model* model = nullptr;

		// Sprite用
		uint32_t textureIndex = 0;
		Vector2 spriteSize = {
			640.0f, 360.0f
		};
	};

	// 配置済みオブジェクトの構造体
	struct PlacedObject {
		RenderObjectType type = RenderObjectType::kModel3D;
		std::string displayName;

		// kModel3Dのときだけ使用
		std::unique_ptr<Object3D> object3D;

		// kSprite2Dのときだけ使用
		std::unique_ptr<Sprite> sprite;
	};

	// 配置済みObject3Dを管理
	std::vector<PlacedObject> placedObjects;

	// Sprite用のテクスチャ
	uint32_t spriteTextureIndex = textureManager.LoadTexture("resources/Textures/uvChecker.png");

	// SphereをModelとして生成
	Model* sphereModel = modelManager.CreateSphereModel("UnitSphere", 1.0f, 16, "uvChecker.png");

	// モデルの読み込み
	std::vector<RenderEntry> renderEntries = {
		{"Sprite", RenderObjectType::kSprite2D,  "", nullptr, spriteTextureIndex,{ 640.0f, 360.0f } },
		{"Sphere", RenderObjectType::kModel3D, "", sphereModel, 0, {} },
		{"Axis", RenderObjectType::kModel3D, "axis.obj", nullptr, 0, {}},
		{"Plane", RenderObjectType::kModel3D, "plane.obj", nullptr, 0, {}},
		{"Bunny", RenderObjectType::kModel3D,  "bunny.obj", nullptr, 0, {}},
		{"Teapot", RenderObjectType::kModel3D, "teapot.obj", nullptr, 0, {}},
	};

	// 各モデルの読み込み
	for (RenderEntry& entry : renderEntries) {
		// modelが未設定の項目だけOBJから読み込む
		if (entry.type == RenderObjectType::kModel3D &&
			entry.model == nullptr) {

			entry.model = modelManager.LoadModel(entry.filename);
		}
	}

	/*-------------------- 音声の初期化 --------------------*/
	// Audioの初期化
	Audio audio;
	audio.Initialize();

	// 音声を管理する構造体
	struct SoundEntry {
		std::string displayName;
		std::string filePath;
		SoundData soundData;
	};

	// 音声読み込み
	std::vector<SoundEntry> soundEntries = {
		{"Fanfare", "resources/Audio/fanfare.wav", {}},
		{"Mokugyo", "resources/Audio/mokugyo.wav", {}},
		{"Start", "resources/Audio/start.wav", {}},
	};

	// 音声ファイルを最初に一度だけ読み込む
	for (SoundEntry& entry : soundEntries) {
		entry.soundData = audio.AudioLoadWave(entry.filePath.c_str());
	}

	// ログ出力
	Log(logStream, "Start logging!");

	/*-------------------- 変数 --------------------*/
	// 選択したモデルを記録
	int selectedModelIndex = 0;
	// spriteを表示するか
	bool showSprite = false;
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

		// 作りたい物に合わせて配置
		// オブジェクトの更新
		for (const auto& placedObject : placedObjects) {
			if (placedObject.type == RenderObjectType::kModel3D) {
				placedObject.object3D->Update(camera);

			} else {
				placedObject.sprite->Update();
			}
		}

		// 音声の更新
		audio.Update();

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
		// Objectを選択
		if (ImGui::BeginCombo("Object Type", renderEntries[selectedModelIndex].displayName.c_str())) {
			// 追加するオブジェクトを選択
			for (int i = 0; i < static_cast<int>(renderEntries.size()); ++i) {
				const bool isSelected = (selectedModelIndex == i);
				// 選択肢を表示
				if (ImGui::Selectable(renderEntries[i].displayName.c_str(), isSelected)) {
					selectedModelIndex = i;
				}
				// 選んだ選択肢をデフォルトフォーカスにする
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		// Objectを追加する
		if (ImGui::Button("Add")) {
			// 現在選択されているモデル情報
			const RenderEntry& entry = renderEntries[selectedModelIndex];

			PlacedObject placedObject{};
			placedObject.type = entry.type;
			placedObject.displayName = entry.displayName;

			// Objectの種類別で追加
			if (entry.type == RenderObjectType::kModel3D) {
				// 3Dモデルの場合
				auto object = std::make_unique<Object3D>();
				// モデルを初期化
				object->Initialize(&dxCommon, &modelPSO, &directionalLight);
				// モデルをセット
				object->SetModel(entry.model);
				// 描画される前にWVPを一度更新する
				object->Update(camera);
				// 
				placedObject.object3D = std::move(object);

			} else {
				// 2Dのスプライトの場合
				auto sprite = std::make_unique<Sprite>();
				// ここは現在のSprite::Initializeの引数に合わせる
				sprite->Initialize(&dxCommon, &spritePSO, &textureManager,
					&winApp, entry.textureIndex, entry.spriteSize);

				// 初期SRT
				sprite->SetScale({ 1.0f,1.0f,1.0f });
				sprite->SetRotate({ 0.0f,0.0f,0.0f });
				sprite->SetTranslate({ 0.0f,0.0f,0.0f });

				// 追加したフレームで正しい行列を使用する
				sprite->Update();
				// 
				placedObject.sprite = std::move(sprite);
			}
			// Object3Dのリスト追加とモデル名の保存を一緒に
			placedObjects.push_back(std::move(placedObject));
		}

		// 削除するモデルを記録する変数
		int deleteObjectIndex = -1;

		// 配置済みモデルの調整
		for (size_t i = 0; i < placedObjects.size(); ++i) {
			// Objectを管理
			PlacedObject& placedObject = placedObjects[i];

			ImGui::PushID(static_cast<int>(i));
			// ラベルを作成
			std::string label = "Object " + std::to_string(i) + " : " + placedObject.displayName;
			// 個別でモデルを調整
			if (ImGui::CollapsingHeader(label.c_str())) {
				// 3Dモデルの場合の処理
				if (placedObject.type == RenderObjectType::kModel3D) {
					Object3D& object = *placedObject.object3D;

					// SRTを取得
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
						// UVTransformを取得
						Transform uvTransform = object.GetUVTransform();
						// UVを変更したか記録
						bool uvChanged = false;
						uvChanged |= ImGui::DragFloat2("UV Scale", &uvTransform.scale.x, 0.01f, 0.01f, 10.0f);	// スケールを変更
						uvChanged |= ImGui::DragFloat2("UV Translate", &uvTransform.translate.x, 0.01f);		// 移動を変更
						uvChanged |= ImGui::DragFloat("UV Rotate", &uvTransform.rotate.z, 0.01f);				// 回転を変更
						// UVで使用しない成分を固定
						if (uvChanged) {
							uvTransform.scale.z = 1.0f;
							uvTransform.rotate.x = 0.0f;
							uvTransform.rotate.y = 0.0f;
							uvTransform.translate.z = 0.0f;
							object.SetUVTransform(uvTransform);
						}

						// テクスチャの色を取得
						Vector4 color = object.GetColor();
						// 色を変更
						if (ImGui::ColorEdit4("Color", &color.x)) {
							object.SetColor(color);
						}

						// ライティングモードを記録する
						int lightingMode = static_cast<int>(object.GetLightingMode());
						// ライティングを変更
						ImGui::Combo("Lighting", &lightingMode, "None\0Lambert\0Half Lambert\0");
						object.SetLightingMode(static_cast<LightingMode>(lightingMode));

						ImGui::TreePop();
					}

				} else {
					// 3DSpriteの場合の処理 
					Sprite& sprite = *placedObject.sprite;

					// SRTを取得
					Vector3 scale = sprite.GetScale();
					Vector3 rotate = sprite.GetRotate();
					Vector3 translate = sprite.GetTranslate();
					// SpriteではXYスケールを使用
					if (ImGui::DragFloat2("Scale", &scale.x, 0.01f, 0.01f, 100.0f)) {
						scale.z = 1.0f;
						sprite.SetScale(scale);
					}
					// SpriteではZ回転を使用
					if (ImGui::DragFloat("Rotate", &rotate.z, 0.01f)) {
						rotate.x = 0.0f;
						rotate.y = 0.0f;
						sprite.SetRotate(rotate);
					}
					// SpriteではXY移動を使用
					if (ImGui::DragFloat2("Translate", &translate.x, 1.0f)) {
						translate.z = 0.0f;
						sprite.SetTranslate(translate);
					}
					// Spriteを削除
					if (ImGui::Button("Delete")) {
						deleteObjectIndex =
							static_cast<int>(i);
					}

					// UV、色を調整
					if (ImGui::TreeNode("Model")) {
						// UVTransformを取得
						Vector3 UVScale = sprite.GetUVTransformScale();
						Vector3 UVTranslate = sprite.GetUVTransformTranslate();
						Vector3 UVRotate = sprite.GetUVTransformRotate();
						if (ImGui::DragFloat2("UV Scale", &UVScale.x, 0.01f, -10.0f, 10.0f)) {			// スケールを変更
							sprite.SetUVTransformScale(UVScale);
						}
						if (ImGui::DragFloat2("UV Translate", &UVTranslate.x, 0.01f, -10.0f, 10.0f)) {	// 移動を変更
							sprite.SetUVTransformTranslate(UVTranslate);
						}
						if (ImGui::SliderAngle("UV Rotate", &UVRotate.z)) {								// 回転を変更
							sprite.SetUVTransformRotate(UVRotate);
						}

						// テクスチャの色を取得
						Vector4 color = sprite.GetColor();
						// 色を変更
						if (ImGui::ColorEdit4("Color", &color.x, ImGuiColorEditFlags_AlphaBar)) {
							sprite.SetColor(color);
						}

						ImGui::TreePop();
					}
				}
			}
			ImGui::PopID();
		}
		// 削除するモデルがあれば削除
		if (deleteObjectIndex >= 0) {
			placedObjects.erase(placedObjects.begin() + deleteObjectIndex);
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
		}

		ImGui::Separator();
		if (ImGui::CollapsingHeader("Audio")) {
			if (soundEntries.empty()) {
				// 音声が無ければ操作項目を非表示に
				ImGui::Text("No sound data");

			} else {
				if (ImGui::BeginCombo("Sound Type", soundEntries[selectedSoundIndex].displayName.c_str())) {
					// 再生する音声を選択
					for (int i = 0; i < static_cast<int>(soundEntries.size()); ++i) {
						const bool isSelected = selectedSoundIndex == i;
						// 選択肢を表示
						if (ImGui::Selectable(soundEntries[i].displayName.c_str(), isSelected)) {
							selectedSoundIndex = i;
						}
						// 選んだ選択肢をデフォルトフォーカスにする
						if (isSelected) {
							ImGui::SetItemDefaultFocus();
						}
					}

					ImGui::EndCombo();
				}

				// Soundの再生
				if (ImGui::Button("Play Sound")) {
					audio.AudioPlayWave(soundEntries[selectedSoundIndex].soundData);
				}

				// 音声の重なりが許可されているか
				bool allowOverlap = audio.GetAllowOverlap();
				// 音声の重なりの許可を切り替える
				ImGui::SameLine();
				if (ImGui::Checkbox("Allow Sound Overlap", &allowOverlap)) {
					audio.SetAllowOverlap(allowOverlap);
				}

				// 再生を即座に停止
				if (ImGui::Button("Stop Sounds")) {
					audio.StopAllSounds();
				}
			}
		}

		imguiManager.End();		// 以上でImGuiの内容を終える
#endif

		// 3Dモデルを描画
		for (const PlacedObject& placedObject : placedObjects) {
			if (placedObject.type == RenderObjectType::kModel3D) {
				placedObject.object3D->Draw();
			}
		}

		// Spriteを後から描画
		for (const PlacedObject& placedObject : placedObjects) {
			if (placedObject.type == RenderObjectType::kSprite2D) {
				placedObject.sprite->Draw();
			}
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
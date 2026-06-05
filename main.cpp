#include <Windows.h>
#include "logger.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "DXCManager.h"
#include "TextureManager.h"
#include "Camera.h"
#include "DirectionalLight.h"
#include "Triangle.h"
#include "TrianglePSO.h"
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

// Triangle演出用
float RandRange(float min, float max) {
	return min + (float)rand() / RAND_MAX * (max - min);
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
	textureManager.LoadTexture("resources/uvChecker.png", 1);
	textureManager.LoadTexture("resources/monsterBall.png", 2);

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
	camera.Initialize(WinApp::kClientWidth_, WinApp::kClientHeight_);

	// 平行光源の初期化
	DirectionalLight directionalLight;
	directionalLight.Initialize(&dxCommon);

	// TrianglePSOの初期化
	TrianglePSO trianglePSO;
	trianglePSO.Initialize(&dxCommon, &dxcManager);

	// Triangleの初期化
	Triangle triangles[2];
	triangles[0].Initialize(&dxCommon, &trianglePSO, &textureManager);
	triangles[1].Initialize(&dxCommon, &trianglePSO, &textureManager);
	triangles[0].SetTextureIndex(1);
	triangles[1].SetTextureIndex(1);
	triangles[0].GetScale() = { 1.0f,1.0f,1.0f };
	triangles[1].GetScale() = { 1.5f,0.8f,1.0f };
	triangles[0].GetRotate() = { 0.0f,0.0f,0.0f };
	triangles[1].GetRotate() = { 0.0f,0.8f,0.0f };
	triangles[0].GetTranslate() = { -0.2f,0.0f,0.0f };
	triangles[1].GetTranslate() = { -0.2f,-0.1f,0.0f };

	// 演出用のTriangleの初期化
	Triangle flyingTriangles[200];
	Vector3 velocity[200];
	Vector3 rotateVelocity[200];
	for (int i = 0; i < 200; i++) {
		flyingTriangles[i].Initialize(&dxCommon, &trianglePSO, &textureManager);
		flyingTriangles[i].SetTextureIndex(1);
		flyingTriangles[i].GetScale() = { 2.0f,2.0f,2.0f };
		flyingTriangles[i].GetRotate() = { RandRange(0.0f, 6.28f),RandRange(0.0f, 6.28f),RandRange(0.0f, 6.28f) };
		flyingTriangles[i].GetTranslate() = { 0.0f,0.0f,RandRange(200.0f, 800.0f) };
		velocity[i] = { RandRange(-0.1f, 0.1f),RandRange(-0.1f, 0.1f),-1.0f };
		rotateVelocity[i] = { RandRange(-0.05f, 0.05f),RandRange(-0.05f, 0.05f),RandRange(-0.05f, 0.05f) };
	}

	// ログ出力
	Log(logStream, "Start logging!");

	/*-------------------- 変数 --------------------*/
	// シーン分け用
	int currentScene = 0;
	const char* items[] = { "TriangleScene", "PerformanceScene" };
	int SceneIndex = 0;

	/*-------------------- ゲームループの処理 --------------------*/
	MSG msg{};
	// ウィンドウのxボタンが押されるまでループ
	while (!winApp.IsEndGame()) {
		winApp.ProcessMessage();

		switch (currentScene) {
		case 0:
			/*==============================================================
			*	ImGuiで2つの三角形を操作
			==============================================================*/

			/*-------------------- 更新処理 --------------------*/
			camera.Update();
			directionalLight.Update();
			for (auto& triangle : triangles) {
				triangle.Update(camera);
			}

			/*-------------------- 描画処理 --------------------*/
			dxCommon.PreDraw();		// 描画前の処理

			// ImGui
#ifdef USE_IMGUI
			imguiManager.Begin();	// 以下にImGuiの内容を記述
			// リストからシーンを変更
			if (ImGui::Combo("Scene", &SceneIndex, items, IM_ARRAYSIZE(items))) {
				currentScene = SceneIndex;
				for (int i = 0; i < 200; i++) {
					flyingTriangles[i].GetRotate() = { RandRange(0.0f, 6.28f),RandRange(0.0f, 6.28f),RandRange(0.0f, 6.28f) };
					flyingTriangles[i].GetTranslate() = { 0.0f,0.0f,RandRange(200.0f, 800.0f) };
					velocity[i] = { RandRange(-0.1f, 0.1f),RandRange(-0.1f, 0.1f),-1.0f };
					rotateVelocity[i] = { RandRange(-0.05f, 0.05f),RandRange(-0.05f, 0.05f),RandRange(-0.05f, 0.05f) };
				}
			}
			// カメラを操作
			if (ImGui::CollapsingHeader("Camera")) {
				// 位置を調整
				ImGui::DragFloat3("Camera.Rotate", &camera.GetRotate().x, 0.01f);
				ImGui::DragFloat3("Camera.Translate", &camera.GetTranslate().x, 0.01f);
			};
			// 三角形を操作
			for (int i = 0; i < 2; i++) {
				std::string headerName = "Triangle" + std::to_string(i + 1);	// ラベル名に番号を加える処理
				if (ImGui::CollapsingHeader(headerName.c_str())) {
					ImGui::PushID(i);
					// リストからテクスチャを選択
					const char* items[] = { "uvChecker", "monsterBall" };
					int textureIndex = static_cast<int>(triangles[i].GetTextureIndex()) - 1;
					if (ImGui::Combo("Texture", &textureIndex, items, IM_ARRAYSIZE(items))) {
						triangles[i].SetTextureIndex(textureIndex + 1);	// Textureは1から始まっているので合わせる
					}
					// color,SRTを変更
					ImGui::ColorEdit4("Color", &triangles[i].GetColor().x);
					ImGui::DragFloat3("Scale", &triangles[i].GetScale().x, 0.01f);
					ImGui::DragFloat3("Rotate", &triangles[i].GetRotate().x, 0.01f);
					ImGui::DragFloat3("Translate", &triangles[i].GetTranslate().x, 0.01f);
					ImGui::PopID();
				}
			}
			imguiManager.End();		// 以上でImGuiの内容を終える
#endif

			// 三角形の描画
			for (auto& triangle : triangles) {
				triangle.Draw();
			}

			// ImGuiの描画
#ifdef USE_IMGUI
			imguiManager.Draw();
#endif

			dxCommon.PostDraw();	// 描画後の処理

			break;
		case 1:
			/*==============================================================
			*	三角形が奥から飛んでくる演出
			==============================================================*/

			/*-------------------- 更新処理 --------------------*/
			camera.Update();
			directionalLight.Update();
			for (int i = 0; i < 200; i++) {
				// 三角形を移動
				flyingTriangles[i].GetTranslate().x += velocity[i].x;
				flyingTriangles[i].GetTranslate().y += velocity[i].y;
				flyingTriangles[i].GetTranslate().z += velocity[i].z;
				// 三角形を回転
				flyingTriangles[i].GetRotate().x += rotateVelocity[i].x;
				flyingTriangles[i].GetRotate().y += rotateVelocity[i].y;
				flyingTriangles[i].GetRotate().z += rotateVelocity[i].z;

				// 初期状態に戻す
				if (flyingTriangles[i].GetTranslate().z < -20.0f) {
					flyingTriangles[i].GetTranslate() = { 0.0f,0.0f,RandRange(200.0f, 500.0f) };
					flyingTriangles[i].GetRotate() = { RandRange(0.0f, 6.28f),RandRange(0.0f, 6.28f),RandRange(0.0f, 6.28f) };
					velocity[i] = { RandRange(-0.1f, 0.1f),RandRange(-0.1f, 0.1f),-1.0f };
					rotateVelocity[i] = { RandRange(-0.05f, 0.05f),RandRange(-0.05f, 0.05f),RandRange(-0.05f, 0.05f) };
				}

				flyingTriangles[i].Update(camera);
			}

			/*-------------------- 描画処理 --------------------*/
			dxCommon.PreDraw();		// 描画前の処理

			// ImGui
#ifdef USE_IMGUI
			imguiManager.Begin();	// 以下にImGuiの内容を記述
			// リストからシーンを変更
			if (ImGui::Combo("Scene", &SceneIndex, items, IM_ARRAYSIZE(items))) {
				currentScene = SceneIndex;
			}
			// カメラを操作
			if (ImGui::CollapsingHeader("Camera")) {
				// 位置を調整
				ImGui::DragFloat3("Camera.Rotate", &camera.GetRotate().x, 0.01f);
				ImGui::DragFloat3("Camera.Translate", &camera.GetTranslate().x, 0.01f);
			};

			imguiManager.End();		// 以上でImGuiの内容を終える
#endif
			// 三角形の描画
			for (auto& triangle : flyingTriangles) {
				triangle.Draw();
			}

			// ImGuiの描画
#ifdef USE_IMGUI
			imguiManager.Draw();
#endif

			dxCommon.PostDraw();	// 描画後の処理

			break;
		}
	}

	// ImGuiの終了処理
#ifdef USE_IMGUI
	imguiManager.Finalize();
#endif
	// COMの終了処理
	CoUninitialize();
	return 0;
}
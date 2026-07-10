#pragma once
#include "struct.h"
#include <cstdint>

namespace Engine {

	// 前方宣言
	class Input;

	class Camera {
	public:
		/*==================== メンバ関数とデストラクタの宣言 ====================*/
		// デストラクタ
		~Camera();

		/// <summary>
		/// Cameraの初期化
		/// </summary>
		/// <param name="width">ウィンドウの幅</param>
		/// <param name="height">ウィンドウの高さ</param>
		/// <param name="input">Inputクラスのインスタンス</param>
		void Initialize(int32_t width, int32_t height, Input* input);
		/// <summary>
		/// Cameraの更新
		/// </summary>
		void Update();

		/*==================== アクセッサ ====================*/
		// ゲッター
		Matrix4x4 GetViewProjectionMatrix() const { return viewProjectionMatrix_; }
		Vector3& GetRotate() { return rotate_; }
		Vector3& GetTranslate() { return translate_; }
		float& GetDistance() { return distance_; }
		bool& GetDebugMode() { return isDebugMode_; }

		// セッター
		void SetDebugMode(bool enable) { isDebugMode_ = enable; }

	private:
		/*==================== メンバ関数の宣言 ====================*/
		// モード別のカメラの更新
		// ゲーム中のカメラ更新
		void UpdateGameCamera();
		// デバッグカメラ更新
		void UpdateDebugCamera();

		/*==================== メンバ変数 ====================*/
		// カメラの変換情報
		Transform transform_;

		Matrix4x4 viewMatrix_;				// ビュー行列(カメラ行列の逆行列)
		Matrix4x4 projectionMatrix_;		// 透視投影行列
		Matrix4x4 viewProjectionMatrix_;	// ビュープロジェクション行列
		// 透視投影行列を作るために、アスペクト比を計算しておく
		float aspectRatio_ = 1.0f;

		// ImGui等で弄る用
		Vector3 rotate_;
		Vector3 translate_;

		// デバッグカメラ用の変数
		// 入力クラスのポインタ
		Input* input_ = nullptr;

		// ターゲットの座標
		Vector3 target_ = { 0.0f,0.0f,0.0f };
		// カメラとターゲットの距離
		float distance_ = 10.0f;
		// カメラの角度
		float orbitYaw_ = 0.0f;		// Y軸回転
		float orbitPitch_ = 0.3f;	// X軸回転
		// デバッグカメラに切り替えているか
		bool isDebugMode_ = false;

		// LookAround用の変数
		float lookYaw_ = 0.0f;		// Y軸回転
		float lookPitch_ = 0.0f;	// X軸回転
		// 視点変更中かどうか
		bool isLookAround_ = false;
	};
}
#pragma once
#include "struct.h"

class Input;

class Camera {
public:
	~Camera();

	void Initialize(float width, float height, Input* input);
	void Update();

	// ゲッター
	Matrix4x4 GetViewProjectionMatrix() const { return viewProjectionMatrix_; }
	Vector3& GetRotate() { return rotate_; }
	Vector3& GetTranslate() { return translate_; }
	bool& GetDebugMode() { return isDebugMode_; }

	// セッター
	void SetDebugMode(bool enable) { isDebugMode_=enable; }

private:
	// モード別のカメラの更新
	void UpdateGameCamera();
	void UpdateDebugCamera();
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
	// ターゲットの座標
	Vector3 target_ = { 0.0f,0.0f,0.0f };
	// カメラとターゲットの距離
	float distance_ = 10.0f;
	// カメラの角度
	float yaw_ = 0.0f;		// Y軸回転
	float pitch_ = 0.3f;	// X軸回転
	// デバッグカメラに切り替えているか
	bool isDebugMode_ = false;

	Input* input_ = nullptr;
};
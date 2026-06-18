#pragma once
#include "struct.h"

class Camera {
public:
	~Camera();

	void Initialize(float width, float height);
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

	Transform transform_;

	Matrix4x4 viewMatrix_;				// ビュー行列(カメラ行列の逆行列)
	Matrix4x4 projectionMatrix_;		// 透視投影行列
	Matrix4x4 viewProjectionMatrix_;	// ビュープロジェクション行列
	// 透視投影行列を作るために、アスペクト比を計算しておく
	float aspectRatio_ = 1.0f;

	// ImGui等で弄る用
	Vector3 rotate_;
	Vector3 translate_;

	// デバッグカメラに切り替えているか
	bool isDebugMode_ = false;
};


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

private:
	Transform transform_;

	Matrix4x4 viewMatrix_;				// ビュー行列(カメラ行列の逆行列)
	Matrix4x4 projectionMatrix_;		// 透視投影行列
	Matrix4x4 viewProjectionMatrix_;	// ビュープロジェクション行列
	// 透視投影行列を作るために、アスペクト比を計算しておく
	float aspectRatio_ = 1.0f;

	// ImGuiで弄る用
	Vector3 rotate_;
	Vector3 translate_;

};


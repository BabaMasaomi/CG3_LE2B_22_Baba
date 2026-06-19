#include "Camera.h"
#include "Input.h"
#include "Utility.h"
#include <algorithm>
#include <Windows.h>

// デストラクタ
Camera::~Camera() {

}

/*-------------------- 初期化 --------------------*/
void Camera::Initialize(float width, float height, Input* input) {
	transform_.scale = { 1.0f,1.0f,1.0f };
	transform_.rotate = { 0.3f,0.0f,0.0f };
	transform_.translate = { 0.0f,4.0f,-10.0f };
	// 透視投影行列を作るために、アスペクト比を計算しておく
	aspectRatio_ = width / height;

	// ImGui用
	rotate_ = transform_.rotate;
	translate_ = transform_.translate;

	input_ = input;
}

/*-------------------- 更新処理 --------------------*/
void Camera::Update() {
	// モード毎に処理を切り替え
	if (isDebugMode_) {
		UpdateDebugCamera();
	} else {
		UpdateGameCamera();
	}

	// カメラの行列を作成
	Matrix4x4 cameraMatrix = MakeAffineMatrix(
		transform_.scale,
		transform_.rotate,
		transform_.translate
	);

	// ビュー行列(カメラ行列の逆行列)
	viewMatrix_ = Inverse(cameraMatrix);
	// 透視投影行列を
	projectionMatrix_ = MakePerspectiveFovMatrix(0.45f, aspectRatio_, 0.1f, 100.0f);
	// ビュープロジェクション行列まで作成、world行列は描画する図形で作り、計算する
	viewProjectionMatrix_ = Multiply(viewMatrix_, projectionMatrix_);
}

/*-------------------- ゲーム中のカメラ更新 --------------------*/
void Camera::UpdateGameCamera() {
	// カメラの変更を適用
	transform_.rotate = rotate_;
	transform_.translate = translate_;
}

/*-------------------- デバッグカメラのカメラ更新 --------------------*/
void Camera::UpdateDebugCamera() {
	// 回転行列を作成
	Matrix4x4 rotateY = MakeRotateYMatrix(yaw_);
	Matrix4x4 rotateX = MakeRotateXMatrix(pitch_);
	Matrix4x4 rotateMatrix = Multiply(rotateX, rotateY);

	// マウスの中ボタンが押されているとき
	if (input_->PushMouseMiddle()) {
		// マウスの移動量を取得
		Vector2 delta = input_->GetMouseDelta();
		// Shiftキーが押されているとき
		if (input_->PushKey(DIK_LSHIFT)) {
			/*--------------- targetを平行移動(Pan) ---------------*/
			Vector3 right = { rotateMatrix.m[0][0],rotateMatrix.m[0][1],rotateMatrix.m[0][2] };
			Vector3 up = { rotateMatrix.m[1][0],rotateMatrix.m[1][1],rotateMatrix.m[1][2] };

			// 距離に応じて速度を調整
			float panSpeed = distance_ * 0.001f;
			// targetを平行移動
			target_ = target_ - right * delta.x * panSpeed + up * delta.y * panSpeed;

		} else {
			/*--------------- targetを中心に回転(Orbit) ---------------*/
			// マウスの移動量から回転角度を計算
			yaw_ += delta.x * 0.01f;
			pitch_ += delta.y * 0.01f;
			// pitchの角度に制限をかける
			const float limit = 1.55f;
			pitch_ = std::clamp(pitch_, -limit, limit);
		}
	}

	/*--------------- 拡大縮小(Dolly) ---------------*/
	// マウスホイールの回転量を取得
	distance_ -= input_->GetWheelDelta() * 0.01f;
	// 距離の制限を設定
	distance_ = std::clamp(distance_, 0.1f, 100.0f);	// 上限
	distance_ = (std::max)(distance_, 0.1f);			// 下限

	/*--------------- Camera自身を回転(LookAround) ---------------*/

	/*--------------- 最後に変更を反映させる ---------------*/
	rotateY = MakeRotateYMatrix(yaw_);
	rotateX = MakeRotateXMatrix(pitch_);
	rotateMatrix = Multiply(rotateX, rotateY);
	// カメラの位置を計算
	Vector3 offset = { 0.0f,0.0f,-distance_ };
	// 回転行列を使ってカメラの位置を回転させる
	offset = TransformNormal(offset, rotateMatrix);
	// ターゲットの位置にオフセットを足してカメラの位置を決定
	transform_.translate = {
		target_.x + offset.x,
		target_.y + offset.y,
		target_.z + offset.z };
	// 回転角度をtransformに反映
	transform_.rotate.x = pitch_;
	transform_.rotate.y = yaw_;
}
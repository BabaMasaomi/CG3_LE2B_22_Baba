#include "Camera.h"
#include "Input.h"
#include "Utility.h"
#include <algorithm>

namespace Engine {

	// デストラクタ
	Camera::~Camera() {

	}

	/*-------------------- 初期化 --------------------*/
	void Camera::Initialize(int32_t width, int32_t height, Input* input) {
		// カメラの変換情報を初期化
		transform_.scale = { 1.0f,1.0f,1.0f };
		transform_.rotate = { 0.3f,0.0f,0.0f };
		transform_.translate = { 0.0f,4.0f,-10.0f };
		distance_ = 10.0f;				// カメラとターゲットの距離を初期化(実数値を入れるのは良くないので、後で変更)
		// デバッグカメラ用の変数を初期化
		orbitYaw_ = 0.0f;
		orbitPitch_ = 0.3f;
		lookYaw_ = 0.0f;
		lookPitch_ = 0.0f;

		// 入力クラスのポインタを保存
		input_ = input;

		// 透視投影行列を作るために、アスペクト比を計算しておく
		aspectRatio_ = float(width) / float(height);

		// ImGui等で弄る用の変数に初期値を入れておく
		rotate_ = transform_.rotate;
		translate_ = transform_.translate;
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
		Matrix4x4 rotateY = MakeRotateYMatrix(orbitYaw_);
		Matrix4x4 rotateX = MakeRotateXMatrix(orbitPitch_);
		Matrix4x4 rotateMatrix = Multiply(rotateX, rotateY);

		// マウスの中ボタンが押されているとき
		if (input_->PushMouseMiddle()) {
			// マウスの移動量を取得
			Vector2 delta = input_->GetMouseDelta();
			// Shiftキーが押されているとき
			if (input_->PushKey(DIK_LSHIFT)) {
				/*--------------- targetを平行移動(Pan) ---------------*/
				// 回転行列から右方向と上方向のベクトルを取得
				Vector3 right = { rotateMatrix.m[0][0],rotateMatrix.m[0][1],rotateMatrix.m[0][2] };
				Vector3 up = { rotateMatrix.m[1][0],rotateMatrix.m[1][1],rotateMatrix.m[1][2] };
				// 距離に応じて速度を調整
				float panSpeed = distance_ * 0.001f;
				// targetを平行移動
				target_ = target_ - right * delta.x * panSpeed + up * delta.y * panSpeed;

			} else {
				/*--------------- targetを中心に回転(Orbit) ---------------*/
				// マウスの移動量から回転角度を計算
				orbitYaw_ += delta.x * 0.01f;
				orbitPitch_ += delta.y * 0.01f;
				// pitchの角度に制限をかける
				const float limit = 1.55f;
				orbitPitch_ = std::clamp(orbitPitch_, -limit, limit);
			}
		}

		/*--------------- 拡大縮小(Dolly) ---------------*/
		// マウスホイールの回転量を取得
		distance_ -= input_->GetWheelDelta() * 0.01f;
		// 距離の制限を設定
		distance_ = std::clamp(distance_, 0.1f, 100.0f);	// 上限
		distance_ = (std::max)(distance_, 0.1f);			// 下限

		/*--------------- Camera自身を回転(LookAround) ---------------*/
		if (input_->PushMouseRight()) {
			isLookAround_ = true;
			// マウスの移動量を取得
			Vector2 delta = input_->GetMouseDelta();
			// マウスの移動量から回転角度を計算
			lookYaw_ += delta.x * 0.005f;
			lookPitch_ += delta.y * 0.005f;
			// pitchの角度に制限をかける
			const float limit = 1.55f;
			lookPitch_ = std::clamp(lookPitch_, -limit, limit);

		} else {
			isLookAround_ = false;
		}

		/*--------------- 最後に変更を反映させる ---------------*/
		// 回転行列を作成
		rotateY = MakeRotateYMatrix(orbitYaw_);
		rotateX = MakeRotateXMatrix(orbitPitch_);
		rotateMatrix = Multiply(rotateX, rotateY);
		if (!isLookAround_) {
			// LookAround中でなければ、ターゲットを中心にカメラの位置を計算
			// カメラの位置を計算
			Vector3 offset = { 0.0f,0.0f,-distance_ };
			// 回転行列を使ってカメラの位置を回転させる
			offset = TransformNormal(offset, rotateMatrix);
			// ターゲットの位置にオフセットを足してカメラの位置を決定
			transform_.translate = {
				target_.x + offset.x,
				target_.y + offset.y,
				target_.z + offset.z
			};
		}
		// 回転角度をtransformに反映
		transform_.rotate.x = orbitPitch_ + lookPitch_;
		transform_.rotate.y = orbitYaw_ + lookYaw_;
	}

	/*-------------------- デバッグカメラを初期状態へ戻す --------------------*/
	void Camera::ResetDebugCamera() {
		// 注視点を初期位置へ戻す
		target_ = { 0.0f,0.0f,0.0f };
		// 注視点との距離を戻す
		distance_ = 10.0f;
		// Orbitの角度を戻す
		orbitYaw_ = 0.0f;
		orbitPitch_ = 0.3f;
		// LookAroundの角度を戻す
		lookYaw_ = 0.0f;
		lookPitch_ = 0.0f;
		isLookAround_ = false;

		// 初期角度から回転行列を作成
		Matrix4x4 rotateY = MakeRotateYMatrix(orbitYaw_);
		Matrix4x4 rotateX = MakeRotateXMatrix(orbitPitch_);
		Matrix4x4 rotateMatrix = Multiply(rotateX, rotateY);

		// 注視点からカメラまでのオフセット
		Vector3 offset = { 0.0f,0.0f,-distance_ };
		offset = TransformNormal(offset, rotateMatrix);

		// デバッグカメラの位置を更新
		transform_.translate = { target_.x + offset.x,target_.y + offset.y,target_.z + offset.z };
		// デバッグカメラの回転を更新
		transform_.rotate = { orbitPitch_,orbitYaw_,0.0f };

		// 通常カメラ用の変数も同期しておく
		rotate_ = transform_.rotate;
		translate_ = transform_.translate;
	}
}
#include "Camera.h"
#include "Utility.h"

// デストラクタ
Camera::~Camera() {

}

/*-------------------- 初期化 --------------------*/
void Camera::Initialize(float width, float height) {
	transform_.scale = { 1.0f,1.0f,1.0f };
	transform_.rotate = { 0.3f,0.0f,0.0f };
	transform_.translate = { 0.0f,4.0f,-10.0f };
	// 透視投影行列を作るために、アスペクト比を計算しておく
	aspectRatio_ = width / height;

	// ImGui用
	rotate_ = transform_.rotate;
	translate_ = transform_.translate;
}

/*-------------------- 更新処理 --------------------*/
void Camera::Update() {
	//// ImGuiの変更を反映する
	//transform_.rotate = rotate_;
	//transform_.translate = translate_;

	// カメラの行列を作成
	Matrix4x4 cameraMatrix = MakeAffineMatrix(
		transform_.scale,
		transform_.rotate,
		transform_.translate
	);

	// モード毎に処理を切り替え
	if (isDebugMode_) {
		UpdateDebugCamera();
	} else{
		UpdateGameCamera();
	}

	// ビュー行列(カメラ行列の逆行列)
	viewMatrix_ = Inverse(cameraMatrix);
	// 透視投影行列を
	projectionMatrix_ = MakePerspectiveFovMatrix(0.45f, aspectRatio_, 0.1f, 100.0f);
	// ビュープロジェクション行列まで作成、world行列は描画する図形で作り、計算する
	viewProjectionMatrix_ = Multiply(viewMatrix_, projectionMatrix_);
}

/*-------------------- ゲーム中のカメラ更新 --------------------*/
void Camera::UpdateGameCamera() {

}

/*-------------------- デバッグカメラのカメラ更新 --------------------*/
void Camera::UpdateDebugCamera() {

}
#include "Object3D.h"
#include "DirectXCommon.h"
#include "ModelPSO.h"
#include "Model.h"
#include "DirectionalLight.h"
#include "Camera.h"
#include "Utility.h"
#include "Logger.h"
#include <format>			// フォーマットの自動指定をしてくれる(+安全策)
#include <cmath>
#include <cassert>
#include <cstring>

namespace Engine {

	// デストラクタ
	Object3D::~Object3D() {
		OutputDebugStringA("~Object3D\n");
	}

	/*-------------------- 初期化 --------------------*/
	void Object3D::Initialize(DirectXCommon* dxCommon, ModelPSO* modelPSO, DirectionalLight* directionalLight) {

		// ポインタを保存しておく
		dxCommon_ = dxCommon;
		modelPSO_ = modelPSO;
		directionalLight_ = directionalLight;

		// CPUで動かす用のTransformを作る
		transform_ = { {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };

		// ImGui用
		scale_ = transform_.scale;
		rotate_ = transform_.rotate;
		translate_ = transform_.translate;

		// 変換行列用のResourceとデータの作成関数
		CreateTransformationMatrixResource();
		MapTransformationMatrix();
	}

	/*-------------------- 変換行列用のResourceの生成 --------------------*/
	void Object3D::CreateTransformationMatrixResource() {
		transformationMatrixResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(TransformationMatrix));
	}

	/*-------------------- 変換行列用のResourceにデータを書き込む --------------------*/
	void Object3D::MapTransformationMatrix() {
		// 書き込むためのアドレスを取得
		HRESULT hr = transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
		assert(SUCCEEDED(hr));
		// 単位行列を書きこんでおく
		transformationMatrixData_->WVP = MakeIdentity4x4();
		transformationMatrixData_->World = MakeIdentity4x4();
	}

	/*-------------------- 更新 --------------------*/
	void Object3D::Update(const Camera& camera) {
		// UVTransform用の行列を作成
		Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransform_.scale);
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransform_.rotate.z));
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransform_.translate));
		model_->UpdateUVTransform(uvTransformMatrix);

		// ImGuiの変更を反映する
		transform_.scale = scale_;
		transform_.rotate = rotate_;
		transform_.translate = translate_;

		// ワールド行列を作成
		worldMatrix_ = MakeAffineMatrix(
			transform_.scale,
			transform_.rotate,
			transform_.translate
		);

		// WVP行列を計算して、CBufferに書き込む
		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix_, camera.GetViewProjectionMatrix());
		transformationMatrixData_->WVP = worldViewProjectionMatrix;
		transformationMatrixData_->World = worldMatrix_;
	}

	/*-------------------- 描画 --------------------*/
	void Object3D::Draw() {
		// コマンドリストを取得
		auto* commandList = dxCommon_->GetCommandList();
		// RootSignature設定
		commandList->SetGraphicsRootSignature(modelPSO_->GetRootSignature());
		commandList->SetPipelineState(modelPSO_->GetPipelineState());	// PSOを設定
		commandList->IASetVertexBuffers(0, 1, &model_->GetVertexBufferView());	// VertexBufferViewを設定
		// 形状を設定
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->SetGraphicsRootConstantBufferView(0, model_->GetMaterialResource()->GetGPUVirtualAddress());	// マテリアルCBufferの場所を設定
		commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());	// wvp用のCBufferの場所を設定
		// SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である
		commandList->SetGraphicsRootDescriptorTable(2, model_->GetTextureHandle());
		commandList->SetGraphicsRootConstantBufferView(3, directionalLight_->GetResource()->GetGPUVirtualAddress());	// 平行光源を適用
		// 描画！(DrawCall)
		commandList->DrawInstanced(UINT(model_->GetModelData().vertices.size()), 1, 0, 0);
	}

	/*-------------------- モデルを設定 --------------------*/
	void Object3D::SetModel(Model* model) {
		// モデルを設定
		model_ = model;
	}
}
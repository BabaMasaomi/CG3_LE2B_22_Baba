#include "Object3d.h"
#include "DirectXCommon.h"
#include "ModelPSO.h"
#include "TextureManager.h"
#include "DirectionalLight.h"
#include "Camera.h"
#include "Utility.h"
#include "Logger.h"
#include <format>			// フォーマットの自動指定をしてくれる(+安全策)
#include <cmath>
#include <cassert>
#include <cstring>

// デストラクタ
Object3d::~Object3d() {
	OutputDebugStringA("~Object3d\n");
}

/*-------------------- 初期化 --------------------*/
void Object3d::Initialize(DirectXCommon* dxCommon, ModelPSO* modelPSO, TextureManager* textureManager, DirectionalLight* directionalLight) {

	// ポインタを保存しておく
	dxCommon_ = dxCommon;
	modelPSO_ = modelPSO;
	textureManager_ = textureManager;
	directionalLight_ = directionalLight;

	// モデルの初期化
	model_.Initialize(dxCommon_,textureManager_);

	// CPUで動かす用のTransformを作る
	transform_ = { {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
	
	// テクスチャの読み込み
	textureIndex_ = 2;
	textureManager_->LoadTexture(model_.GetModelData().material.textureFilePath, textureIndex_);

	// ImGui用
	scale_ = transform_.scale;
	rotate_ = transform_.rotate;
	translate_ = transform_.translate;

	// 色付けのためのMaterialResourceも作る
	CreateMaterialResource();
	MapMaterial();

	// 変換行列用のResourceとデータの作成関数
	CreateTransformationMatrixResource();
	MapTransformationMatrix();
}

/*-------------------- MaterialResourceの生成 --------------------*/
void Object3d::CreateMaterialResource() {
	materialResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(Material));
}

/*-------------------- MaterialResourceにデータを書き込む --------------------*/
void Object3d::MapMaterial() {
	// Mapする
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	std::memset(materialData_, 0, sizeof(Material));
	// ここで色を弄れる
	materialData_->color = { model_.GetModelData().material.diffuseColor.x, model_.GetModelData().material.diffuseColor.y, model_.GetModelData().material.diffuseColor.z, 1.0f };
	materialData_->enableLighting = true;
	materialData_->uvTransform = MakeIdentity4x4();
}

/*-------------------- 変換行列用のResourceの生成 --------------------*/
void Object3d::CreateTransformationMatrixResource() {
	transformationMatrixResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(TransformationMatrix));
}

/*-------------------- 変換行列用のResourceにデータを書き込む --------------------*/
void Object3d::MapTransformationMatrix() {
	// 書き込むためのアドレスを取得
	HRESULT hr = transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	assert(SUCCEEDED(hr));
	// 単位行列を書きこんでおく
	transformationMatrixData_->WVP = MakeIdentity4x4();
	transformationMatrixData_->World = MakeIdentity4x4();
}

/*-------------------- 更新 --------------------*/
void Object3d::Update(const Camera& camera) {
	// UVTransform用の行列を作成
	Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransform_.scale);
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransform_.rotate.z));
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransform_.translate));
	materialData_->uvTransform = uvTransformMatrix;

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
void Object3d::Draw() {
	// コマンドリストを取得
	auto* commandList = dxCommon_->GetCommandList();
	// RootSignature設定
	commandList->SetGraphicsRootSignature(modelPSO_->GetRootSignature());
	commandList->SetPipelineState(modelPSO_->GetPipelineState());	// PSOを設定
	commandList->IASetVertexBuffers(0, 1, &model_.GetVertexBufferView());		// VertexBufferViewを設定
	commandList->IASetIndexBuffer(&model_.GetIndexBufferView());				// IndexBufferViewを設定
	// 形状を設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());				// マテリアルCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());	// wvp用のCBufferの場所を設定
	// SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である
	commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandleGPU(textureIndex_ - 1));
	commandList->SetGraphicsRootConstantBufferView(3, directionalLight_->GetResource()->GetGPUVirtualAddress());	// 平行光源を適用
	// 描画！(DrawCall)
	commandList->DrawInstanced(UINT(model_.GetModelData().vertices.size()), 1, 0, 0);
}
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

	// CPUで動かす用のTransformを作る
	transform_ = { {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };

	// モデル読み込み
	modelData_ = model_->LoadObjFile("resources", "axis.obj");
	
	// テクスチャの読み込み
	textureIndex_ = 2;
	textureManager_->LoadTexture(modelData_.material.textureFilePath, textureIndex_);

	// ImGui用
	scale_ = transform_.scale;
	rotate_ = transform_.rotate;
	translate_ = transform_.translate;

	// VertexResourceとVertexBufferViewを用意
	CreateVertexResource();
	CreateVertexBufferView();
	MapVertexData();

	// IndexResourceとIndexBufferViewの作成関数
	CreateIndexResource();
	CreateIndexBufferView();
	MapIndexData();

	// 色付けのためのMaterialResourceも作る
	CreateMaterialResource();
	MapMaterial();

	// 変換行列用のResourceとデータの作成関数
	CreateTransformationMatrixResource();
	MapTransformationMatrix();
}

/*-------------------- VertexResourceの生成 --------------------*/
void Object3d::CreateVertexResource() {
	vertexResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * modelData_.vertices.size());
}

/*-------------------- VertexBufferViewの作成 --------------------*/
void Object3d::CreateVertexBufferView() {
	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはモデルデータに依存
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
}

/*-------------------- VertexResourceにデータを書き込む --------------------*/
void Object3d::MapVertexData() {
	// 頂点リソースにデータを書きこむ
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// 頂点データをリソースにコピー
	std::memcpy(vertexData, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
}

/*-------------------- IndexResourceの生成 --------------------*/
void Object3d::CreateIndexResource() {
	indexCount_ = kSubdivision_ * kSubdivision_ * 6;
	indexResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(uint32_t) * indexCount_);
}

/*-------------------- IndexBufferViewの作成 --------------------*/
void Object3d::CreateIndexBufferView() {
	// リソースの先頭のアドレスから使う
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックスの数に依存
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * indexCount_;
	// 1頂点あたりのサイズ
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
}

/*-------------------- IndexResourceにデータを書き込む --------------------*/
void Object3d::MapIndexData() {
	// 書き込むためのアドレスを取得
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
	uint32_t i = 0;
	// 緯度と経度のループ
	for (uint32_t lat = 0; lat < kSubdivision_; lat++) {
		for (uint32_t lon = 0; lon < kSubdivision_; lon++) {
			uint32_t current = lat * (kSubdivision_ + 1) + lon;
			uint32_t next = current + 1;
			uint32_t down = current + (kSubdivision_ + 1);
			uint32_t downNext = down + 1;
			// データを設定
			indexData_[i++] = current;
			indexData_[i++] = down;
			indexData_[i++] = next;
			indexData_[i++] = next;
			indexData_[i++] = down;
			indexData_[i++] = downNext;
		}
	}
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
	materialData_->color = { modelData_.material.diffuseColor.x, modelData_.material.diffuseColor.y, modelData_.material.diffuseColor.z, 1.0f };
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
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);		// VertexBufferViewを設定
	commandList->IASetIndexBuffer(&indexBufferView_);				// IndexBufferViewを設定
	// 形状を設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());				// マテリアルCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());	// wvp用のCBufferの場所を設定
	// SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である
	commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandleGPU(textureIndex_ - 1));
	commandList->SetGraphicsRootConstantBufferView(3, directionalLight_->GetResource()->GetGPUVirtualAddress());	// 平行光源を適用
	// 描画！(DrawCall)
	commandList->DrawInstanced(UINT(modelData_.vertices.size()), 1, 0, 0);
}
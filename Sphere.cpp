#include "Sphere.h"
#include "DirectXCommon.h"
#include "SpherePSO.h"
#include "TextureManager.h"
#include "Camera.h"
#include "Utility.h"
#include <cmath>
#include <cassert>

// デストラクタ
Sphere::~Sphere() {
	if (vertexResource_) vertexResource_->Release();
	if (materialResource_) materialResource_->Release();
	if (transformationMatrixResource_) {
		transformationMatrixResource_->Release();
	}
}

/*-------------------- 初期化 --------------------*/
void Sphere::Initialize(DirectXCommon* dxCommon, SpherePSO* spherePSO, TextureManager* textureManager) {

	// DirectXCommonのポインタを保存しておく
	dxCommon_ = dxCommon;
	spherePSO_ = spherePSO;
	textureManager_ = textureManager;

	// CPUで動かす用のTransformを作る
	transform_ = { {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };

	// VertexResourceとVertexBufferViewを用意
	CreateVertexResource();
	CreateVertexBufferView();
	MapVertexData();

	// 色付けのためのMaterialResourceも作る
	CreateMaterialResource();
	MapMaterial();

	// 変換行列用のResourceとデータの作成関数
	CreateTransformationMatrixResource();
	MapTransformationMatrix();
}

/*-------------------- VertexResourceの生成 --------------------*/
void Sphere::CreateVertexResource() {
	vertexResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * vertexCount_);
}

/*-------------------- VertexBufferViewの作成 --------------------*/
void Sphere::CreateVertexBufferView() {
	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはVector4を球の頂点数分
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * vertexCount_;
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
}

/*-------------------- Resourceにデータを書き込む --------------------*/
void Sphere::MapVertexData() {
	// 頂点リソースにデータを書きこむ
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	const float pi = std::acos(-1.0f);
	// 経度1つ分の角度
	const float kLonEvery = pi * 2.0f / float(kSubdivision_);
	// 緯度1つ分の角度
	const float kLatEvery = pi / float(kSubdivision_);
	// 球(大量の三角形)のデータを設定
	// 緯度の方向に分割
	for (uint32_t latIndex = 0; latIndex < kSubdivision_; ++latIndex) {
		float lat0 = -pi / 2.0f + kLatEvery * latIndex;
		float lat1 = lat0 + kLatEvery;
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision_; ++lonIndex) {
			float lon0 = lonIndex * kLonEvery;
			float lon1 = lon0 + kLonEvery;
			uint32_t start = (latIndex * kSubdivision_ + lonIndex) * 6;
			// 頂点にデータを入力する
			// 4点計算
			Vector4 p0 = {
				std::cos(lat0) * std::cos(lon0),
				std::sin(lat0),
				std::cos(lat0) * std::sin(lon0),
				1.0f };
			Vector4 p1 = {
				std::cos(lat1) * std::cos(lon0),
				std::sin(lat1),
				std::cos(lat1) * std::sin(lon0),
				1.0f };
			Vector4 p2 = {
				std::cos(lat0) * std::cos(lon1),
				std::sin(lat0),
				std::cos(lat0) * std::sin(lon1)
				,1.0f };
			Vector4 p3 = {
				std::cos(lat1) * std::cos(lon1)
				,std::sin(lat1)
				,std::cos(lat1) * std::sin(lon1),
				1.0f };
			// UV計算
			float u0 = float(lonIndex) / float(kSubdivision_);
			float u1 = float(lonIndex + 1) / float(kSubdivision_);

			float v0 = 1.0f - float(latIndex) / float(kSubdivision_);
			float v1 = 1.0f - float(latIndex + 1) / float(kSubdivision_);
			// 6頂点書き込み
			// triangle 1
			vertexData[start + 0].pos = p0;
			vertexData[start + 0].texcoord = { u0, v0 };
			vertexData[start + 1].pos = p1;
			vertexData[start + 1].texcoord = { u0, v1 };
			vertexData[start + 2].pos = p2;
			vertexData[start + 2].texcoord = { u1, v0 };
			// triangle 2
			vertexData[start + 3].pos = p2;
			vertexData[start + 3].texcoord = { u1, v0 };
			vertexData[start + 4].pos = p1;
			vertexData[start + 4].texcoord = { u0, v1 };
			vertexData[start + 5].pos = p3;
			vertexData[start + 5].texcoord = { u1, v1 };
		}
	}
}

/*-------------------- MaterialResourceの生成 --------------------*/
void Sphere::CreateMaterialResource() {
	materialResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(Vector4));
}

/*-------------------- MaterialResourceにデータを書き込む --------------------*/
void Sphere::MapMaterial() {
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	// ここで色を弄れる
	*materialData_ = { 1.0f, 1.0f, 1.0f, 1.0f };
}

/*-------------------- 変換行列用のResourceの生成 --------------------*/
void Sphere::CreateTransformationMatrixResource() {
	transformationMatrixResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(TransformationMatrix));
}

/*-------------------- 変換行列用のResourceにデータを書き込む --------------------*/
void Sphere::MapTransformationMatrix() {
	// 書き込むためのアドレスを取得
	HRESULT hr = transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	assert(SUCCEEDED(hr));
	// 単位行列を書きこんでおく
	transformationMatrixData_->WVP = MakeIdentity4x4();
}

/*-------------------- 更新 --------------------*/
void Sphere::Update(const Camera& camera) {
	transform_.rotate.y += 0.02f;

	worldMatrix_ = MakeAffineMatrix(
		transform_.scale,
		transform_.rotate,
		transform_.translate
	);

	// WVP行列を計算して、CBufferに書き込む
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix_, camera.GetViewProjectionMatrix());
	transformationMatrixData_->WVP = worldViewProjectionMatrix;
}

/*-------------------- 描画 --------------------*/
void Sphere::Draw() {
	auto* commandList = dxCommon_->GetCommandList();
	// RootSignature設定
	commandList->SetGraphicsRootSignature(spherePSO_->GetRootSignature());
	// PSO設定
	commandList->SetPipelineState(spherePSO_->GetPipelineState());
	// 形状を設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// VertexBufferViewを設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	// TransformationMatrixCBufferの場所を指定
	commandList->SetGraphicsRootConstantBufferView(0, transformationMatrixResource_->GetGPUVirtualAddress());
	// 1 : Material
	commandList->SetGraphicsRootConstantBufferView(1, materialResource_->GetGPUVirtualAddress());
	// 2 : Texture
	commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandleGPU());
	// 描画！(DrawCall)
	commandList->DrawInstanced(vertexCount_, 1, 0, 0);
}
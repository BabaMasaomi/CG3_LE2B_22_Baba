#include "Sprite.h"
#include "DirectXCommon.h"
#include "ModelPSO.h"
#include "WinApp.h"
#include "TextureManager.h"
#include "Utility.h"
#include <cstring>

namespace Engine {

	// デストラクタ
	Sprite::~Sprite() {
		OutputDebugStringA("~Sprite\n");
	}

	/*-------------------- 初期化 --------------------*/
	void Sprite::Initialize(DirectXCommon* dxCommon, ModelPSO* modelPSO, TextureManager* textureManager, WinApp* winApp) {

		// DirectXCommonのポインタを保存しておく
		dxCommon_ = dxCommon;
		modelPSO_ = modelPSO;
		textureManager_ = textureManager;
		winApp_ = winApp;

		// CPUで動かす用のTransformを作る
		transform_ = { {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };

		// テクスチャの読み込み
		textureIndex_ = 1;
		textureManager_->LoadTexture("resources/uvChecker.png");

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
	void Sprite::CreateVertexResource() {
		vertexResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * 4);
	}

	/*-------------------- VertexBufferViewの作成 --------------------*/
	void Sprite::CreateVertexBufferView() {
		// リソースの先頭のアドレスから使う
		vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
		// 使用するリソースのサイズはVector4を6頂点分
		vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
		// 1頂点あたりのサイズ
		vertexBufferView_.StrideInBytes = sizeof(VertexData);
	}

	/*-------------------- VertexResourceにデータを書き込む --------------------*/
	void Sprite::MapVertexData() {
		// 頂点リソースにデータを書きこむ
		VertexData* vertexData = nullptr;
		// 書き込むためのアドレスを取得
		vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
		// 四角形(二つの三角形)のデータを設定
		vertexData[0].pos = { 0.0f, 360.0f, 0.0f, 1.0f };	// 左下
		vertexData[0].texcoord = { 0.0f, 1.0f };
		vertexData[0].normal = { 0.0f,0.0f,-1.0f };
		vertexData[1].pos = { 0.0f, 0.0f, 0.0f, 1.0f };		// 左上
		vertexData[1].texcoord = { 0.0f, 0.0f };
		vertexData[1].normal = { 0.0f,0.0f,-1.0f };
		vertexData[2].pos = { 640.0f, 360.0f, 0.0f, 1.0f };	// 右下
		vertexData[2].texcoord = { 1.0f, 1.0f };
		vertexData[2].normal = { 0.0f,0.0f,-1.0f };
		vertexData[3].pos = { 640.0f, 0.0f, 0.0f, 1.0f };		// 右上
		vertexData[3].texcoord = { 1.0f, 0.0f };
		vertexData[3].normal = { 0.0f,0.0f,-1.0f };
	}

	/*-------------------- IndexResourceの生成 --------------------*/
	void Sprite::CreateIndexResource() {
		indexResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(uint32_t) * 6);
	}

	/*-------------------- IndexBufferViewの作成 --------------------*/
	void Sprite::CreateIndexBufferView() {
		// リソースの先頭のアドレスから使う
		indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
		// 使用するリソースのサイズはVector4を6頂点分
		indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
		// 1頂点あたりのサイズ
		indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
	}

	/*-------------------- IndexResourceにデータを書き込む --------------------*/
	void Sprite::MapIndexData() {
		// IndexResourceにデータを書き込む
		uint32_t* indexData = nullptr;
		// 書き込むためのアドレスを取得
		indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
		// データを設定
		indexData[0] = 0;
		indexData[1] = 1;
		indexData[2] = 2;
		indexData[3] = 1;
		indexData[4] = 3;
		indexData[5] = 2;
	}

	/*-------------------- MaterialResourceの生成 --------------------*/
	void Sprite::CreateMaterialResource() {
		materialResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(Material));
	}

	/*-------------------- MaterialResourceにデータを書き込む --------------------*/
	void Sprite::MapMaterial() {
		// Mapする
		materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
		std::memset(materialData_, 0, sizeof(Material));
		// ここで色を弄れる
		materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
		materialData_->enableLighting = false;
		materialData_->uvTransform = MakeIdentity4x4();
	}

	/*-------------------- 変換行列用のResourceの生成 --------------------*/
	void Sprite::CreateTransformationMatrixResource() {
		transformationMatrixResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(Matrix4x4));
	}

	/*-------------------- 変換行列用のResourceにデータを書き込む --------------------*/
	void Sprite::MapTransformationMatrix() {
		// 書き込むためのアドレスを取得
		transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
		// 単位行列を書きこんでおく
		transformationMatrixData_->WVP = MakeIdentity4x4();
	}

	/*-------------------- 更新 --------------------*/
	void Sprite::Update() {
		// UVTransform用の行列を作成
		Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransform_.scale);
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransform_.rotate.z));
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransform_.translate));
		materialData_->uvTransform = uvTransformMatrix;

		// Sprite用のWorldViewProjectionMatrixを作る
		Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
		Matrix4x4 viewMatrix = MakeIdentity4x4();
		Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, float(winApp_->kClientWidth_), float(winApp_->kClientHeight_), 0.0f, 100.0f);
		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
		transformationMatrixData_->WVP = worldViewProjectionMatrix;
	}

	/*-------------------- 描画 --------------------*/
	void Sprite::Draw() {
		auto* commandList = dxCommon_->GetCommandList();
		// RootSignature設定
		commandList->SetGraphicsRootSignature(modelPSO_->GetRootSignature());
		commandList->SetPipelineState(modelPSO_->GetSpritePipelineState());
		// 形状を設定
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// VertexBufferViewを設定
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
		// IBVを設定
		commandList->IASetIndexBuffer(&indexBufferView_);
		// マテリアルのCBufferの場所を設定
		commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
		// TransformationMatrixCBufferの場所を指定
		commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
		// SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である
		commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandleGPU(textureIndex_ - 1));
		// 描画！(DrawCall)
		commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	}
}
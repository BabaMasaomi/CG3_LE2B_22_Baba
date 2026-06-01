#include "Sprite.h"
#include "DirectXCommon.h"
#include "WinApp.h"
#include "TextureManager.h"
#include "Utility.h"

// デストラクタ
Sprite::~Sprite() {
	if (vertexResource_) vertexResource_->Release();
	if (transformationMatrixResource_) {
		transformationMatrixResource_->Release();
	}
}

/*-------------------- 初期化 --------------------*/
void Sprite::Initialize(DirectXCommon* dxCommon, TextureManager* textureManager, WinApp* winApp) {

	// DirectXCommonのポインタを保存しておく
	dxCommon_ = dxCommon;
	textureManager_ = textureManager;
	winApp_ = winApp;

	// CPUで動かす用のTransformを作る
	transform_ = { {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };

	// VertexResourceとVertexBufferViewを用意
	CreateVertexResource();
	CreateVertexBufferView();
	MapVertexData();

	// 変換行列用のResourceとデータの作成関数
	CreateTransformationMatrixResource();
	MapTransformationMatrix();
}

/*-------------------- VertexResourceの生成 --------------------*/
void Sprite::CreateVertexResource() {
	vertexResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * 6);
}

/*-------------------- VertexBufferViewの作成 --------------------*/
void Sprite::CreateVertexBufferView() {
	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはVector4を6頂点分
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 6;
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
}

/*-------------------- Resourceにデータを書き込む --------------------*/
void Sprite::MapVertexData() {
	// 頂点リソースにデータを書きこむ
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// 四角形(二つの三角形)のデータを設定
	// 1枚目の三角形
	vertexData[0].pos = { 0.0f, 360.0f, 0.0f, 1.0f };	// 左下
	vertexData[0].texcoord = { 0.0f, 1.0f };
	vertexData[0].normal = { 0.0f,0.0f,-1.0f };
	vertexData[1].pos = { 0.0f, 0.0f, 0.0f, 1.0f };		// 左上
	vertexData[1].texcoord = { 0.0f, 0.0f };
	vertexData[1].normal = { 0.0f,0.0f,-1.0f };
	vertexData[2].pos = { 640.0f, 360.0f, 0.0f, 1.0f };	// 右下
	vertexData[2].texcoord = { 1.0f, 1.0f };
	vertexData[2].normal = { 0.0f,0.0f,-1.0f };
	// 2枚目の三角形
	vertexData[3].pos = { 0.0f, 0.0f, 0.0f, 1.0f };		// 左上
	vertexData[3].texcoord = { 0.0f, 0.0f };
	vertexData[3].normal = { 0.0f,0.0f,-1.0f };
	vertexData[4].pos = { 640.0f, 0.0f, 0.0f, 1.0f };	// 右上
	vertexData[4].texcoord = { 1.0f, 0.0f };
	vertexData[4].normal = { 0.0f,0.0f,-1.0f };
	vertexData[5].pos = { 640.0f, 360.0f, 0.0f, 1.0f };	// 右下
	vertexData[5].texcoord = { 1.0f, 1.0f };
	vertexData[5].normal = { 0.0f,0.0f,-1.0f };
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
	// Sprite用のWorldViewProjectionMatrixを作る
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	Matrix4x4 viewMatrix = MakeIdentity4x4();
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, float(winApp_->kClientWidth_), float(winApp_->kClientHeight_), 0.0f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData_->WVP = worldViewProjectionMatrix;
}

/*-------------------- 描画 --------------------*/
void Sprite::Draw(uint32_t textureIndex) {
	auto* commandList = dxCommon_->GetCommandList();
	// 形状を設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// VertexBufferViewを設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	// TransformationMatrixCBufferの場所を指定
	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である
	commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandleGPU(textureIndex - 1));
	// 描画！(DrawCall)
	commandList->DrawInstanced(6, 1, 0, 0);
}
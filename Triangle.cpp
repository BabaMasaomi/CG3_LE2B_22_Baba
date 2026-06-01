#include "Triangle.h"
#include "DirectXCommon.h"
#include "TrianglePSO.h"
#include "TextureManager.h"
#include "Camera.h"
#include "Utility.h"
#include <cassert>


// デストラクタ
Triangle::~Triangle() {
	if (vertexResource_) vertexResource_->Release();
	if (materialResource_) materialResource_->Release();
	if (wvpResource_) wvpResource_->Release();
}

/*-------------------- 初期化 --------------------*/
void Triangle::Initialize(DirectXCommon* dxCommon, TrianglePSO* trianglePSO, TextureManager* textureManager) {
	// 図形の変換を管理するためのTransformとワールド行列
	transform_.scale = { 1.0f,1.0f,1.0f };
	transform_.rotate = { 0.0f,0.0f,0.0f };
	transform_.translate = { 0.0f,0.0f,0.0f };
	worldMatrix_ = {};

	// DirectXCommonとTrianglePSOのポインタを保存しておく
	dxCommon_ = dxCommon;
	trianglePSO_ = trianglePSO;
	textureManager_ = textureManager;

	// 頂点リソースの生成と、VertexBufferViewの作成、頂点データの書き込み
	CreateVertexResource();
	CreateVertexBufferView();
	MapVertexData();

	// 色付けのためのMaterialResourceも作る
	CreateMaterialResource();
	MapMaterial();

	// 変換行列用のResourceも作る
	CreateTransformationMatrixResource();
	MapTransformationMatrix();
}

/*-------------------- VertexResourceの生成 --------------------*/
void Triangle::CreateVertexResource() {
	vertexResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * 6);
}

/*-------------------- VertexBufferViewの作成 --------------------*/
void Triangle::CreateVertexBufferView() {
	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはVector4を3頂点分
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 6;
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
}

/*-------------------- Resourceにデータを書き込む --------------------*/
void Triangle::MapVertexData() {
	// 頂点リソースにデータを書きこむ
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// 三角形の頂点データを設定
	// 左下
	vertexData[0].pos = { -0.5f, -0.5f, 0.0f, 1.0f };
	vertexData[0].texcoord = { 0.0f,1.0f };
	// 上
	vertexData[1].pos = { 0.0f, 0.5f, 0.0f, 1.0f };
	vertexData[1].texcoord = { 0.5f,0.0f };
	// 右下
	vertexData[2].pos = { 0.5f, -0.5f, 0.0f, 1.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };
	// 2つ目の三角形
	// 左下
	vertexData[3].pos = { -0.5f, -0.5f, 0.5f, 1.0f };
	vertexData[3].texcoord = { 0.0f,1.0f };
	// 上
	vertexData[4].pos = { 0.0f, 0.0f, 0.0f, 1.0f };
	vertexData[4].texcoord = { 0.5f,0.0f };
	// 右下
	vertexData[5].pos = { 0.5f, -0.5f, -0.5f, 1.0f };
	vertexData[5].texcoord = { 1.0f,1.0f };
}

/*-------------------- MaterialResourceの生成 --------------------*/
void Triangle::CreateMaterialResource() {
	materialResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(Vector4));
}

/*-------------------- MaterialResourceにデータを書き込む --------------------*/
void Triangle::MapMaterial() {
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	// ここで色を弄れる
	*materialData_ = { 1.0f, 1.0f, 1.0f, 1.0f };
}

/*-------------------- 変換行列用のResourceの生成 --------------------*/
void Triangle::CreateTransformationMatrixResource() {
	wvpResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(Matrix4x4));
}

/*-------------------- 変換行列用のResourceにデータを書き込む --------------------*/
void Triangle::MapTransformationMatrix() {
	// 書き込むためのアドレスを取得
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpMatrixData_));
	// 単位行列を書きこんでおく
	wvpMatrixData_->WVP = MakeIdentity4x4();
}

/*-------------------- 更新 --------------------*/
void Triangle::Update(const Camera& camera) {
	transform_.rotate.y += 0.02f;

	// 回転を加える
	worldMatrix_ = MakeAffineMatrix(
		transform_.scale,
		transform_.rotate,
		transform_.translate
	);

	// WVP行列を計算して、CBufferに書き込む
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix_, camera.GetViewProjectionMatrix());
	wvpMatrixData_->WVP = worldViewProjectionMatrix;
}

/*-------------------- 描画 --------------------*/
void Triangle::Draw(uint32_t textureIndex) {
	assert(dxCommon_ != nullptr);
	assert(trianglePSO_ != nullptr);
	assert(trianglePSO_->GetRootSignature() != nullptr);
	assert(trianglePSO_->GetPipelineState() != nullptr);

	// コマンドリストを取得(クラス化により必要に)
	auto commandList = dxCommon_->GetCommandList();
	commandList->RSSetViewports(1, &dxCommon_->GetViewPort());			// ViewPortを設定
	commandList->RSSetScissorRects(1, &dxCommon_->GetScissorRect());	// ScissorRectを設定
	// RootSignatureを設定。PSOとは別途設定する必要があることに注意
	commandList->SetGraphicsRootSignature(trianglePSO_->GetRootSignature());
	commandList->SetPipelineState(trianglePSO_->GetPipelineState());	// PSOを設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);			// VertexBufferViewを設定
	// 形状を設定。こちらもPSOとは別途設定する必要がある
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());	// マテリアルCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());		// wvp用のCBufferの場所を設定
	// SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である
	commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandleGPU(textureIndex - 1));
	// 描画！(DrawCall)3頂点で一つのインスタンス
	commandList->DrawInstanced(6, 1, 0, 0);
}
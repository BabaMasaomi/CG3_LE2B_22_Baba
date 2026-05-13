#include "Triangle.h"
#include "DirectXCommon.h"
#include "TrianglePSO.h"
#include "Utility.h"
#include <cassert>


// デストラクタ
Triangle::~Triangle() {
	if (vertexResource_) vertexResource_->Release();
	if (materialResource_) materialResource_->Release();
	if (wvpResource_) wvpResource_->Release();
}


/*-------------------- 初期化 --------------------*/
void Triangle::Initialize(DirectXCommon* dxCommon, TrianglePSO* trianglePSO) {
	dxCommon_ = dxCommon;
	trianglePSO_ = trianglePSO;

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
	vertexResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(Vector4) * 3);
}

/*-------------------- VertexBufferViewの作成 --------------------*/
void Triangle::CreateVertexBufferView() {
	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはVector4を3頂点分
	vertexBufferView_.SizeInBytes = sizeof(Vector4) * 3;
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(Vector4);
}

/*-------------------- Resourceにデータを書き込む --------------------*/
void Triangle::MapVertexData() {
	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	// 三角形の頂点データを設定
	// 左下
	vertexData_[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
	// 上
	vertexData_[1] = { 0.0f, 0.5f, 0.0f, 1.0f };
	// 右下
	vertexData_[2] = { 0.5f, -0.5f, 0.0f, 1.0f };
}

/*-------------------- MaterialResourceの生成 --------------------*/
void Triangle::CreateMaterialResource() {
	materialResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(Vector4));
}

/*-------------------- MaterialResourceにデータを書き込む --------------------*/
void Triangle::MapMaterial() {
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	*materialData_ = { 1.0f, 0.0f, 0.0f, 1.0f }; // 赤
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
	*wvpMatrixData_ = MakeIdentity4x4();
}

/*-------------------- 描画 --------------------*/
void Triangle::Draw() {
	assert(dxCommon_ != nullptr);
	assert(trianglePSO_ != nullptr);
	assert(trianglePSO_->GetRootSignature() != nullptr);
	assert(trianglePSO_->GetPipelineState() != nullptr);

	// コマンドリストを取得(クラス化により必要に)
	auto commandList = dxCommon_->GetCommandList();
	commandList->RSSetViewports(1, &dxCommon_->GetViewPort());										// ViewPortを設定
	commandList->RSSetScissorRects(1, &dxCommon_->GetScissorRect());								// ScissorRectを設定
	// RootSignatureを設定。PSOとは別途設定する必要があることに注意
	commandList->SetGraphicsRootSignature(trianglePSO_->GetRootSignature());
	commandList->SetPipelineState(trianglePSO_->GetPipelineState());								// PSOを設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);										// VertexBufferViewを設定
	// 形状を設定。こちらもPSOとは別途設定する必要がある
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());	// マテリアルCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());		// wvp用のCBufferの場所を設定
	// 描画！(DrawCall)3頂点で一つのインスタンス
	commandList->DrawInstanced(3, 1, 0, 0);
}

///*-------------------- BufferResourceの作成 --------------------*/
//ID3D12Resource* Triangle::CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
//	HRESULT hr;
//
//	// 頂点リソース用のヒープを生成
//	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
//	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;		// UploadHeapを使う
//	// 頂点リソースの設定
//	D3D12_RESOURCE_DESC resourceDesc{};
//	// バッファリソース。テクスチャの場合は別の設定をする
//	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
//	resourceDesc.Width = sizeInBytes;						// リソースのサイズ。引数で指定
//	// バッファの場合はこれらは全て1にする
//	resourceDesc.Height = 1;
//	resourceDesc.DepthOrArraySize = 1;
//	resourceDesc.MipLevels = 1;
//	resourceDesc.SampleDesc.Count = 1;
//	// バッファの場合はこれにする
//	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
//	// 返り値を用意
//	ID3D12Resource* resource = nullptr;
//	// 実際に頂点リソースを作る
//	hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
//		&resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
//		IID_PPV_ARGS(&resource));
//	assert(SUCCEEDED(hr));
//
//	return resource;
//}
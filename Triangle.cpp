#include "Triangle.h"
#include "DirectXCommon.h"
#include "TrianglePSO.h"
#include <cassert>


// デストラクタ
Triangle::~Triangle() {
	if (vertexResource_) vertexResource_->Release();
}

/*-------------------- 初期化 --------------------*/
void Triangle::Initialize(DirectXCommon* dxCommon, TrianglePSO* trianglePSO) {
	dxCommon_ = dxCommon;
	trianglePSO_ = trianglePSO;

	CreateVertexResource();
	CreateVertexBufferView();
	MapVertexData();
}

/*-------------------- VertexResourceの生成 --------------------*/
void Triangle::CreateVertexResource() {
	HRESULT hr;

	// 頂点リソース用のヒープを生成
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;		// UploadHeapを使う
	// 頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	// バッファリソース。テクスチャの場合は別の設定をする
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeof(Vecter4) * 3;			// リソースのサイズ。今回はVecter4を3頂点分
	// バッファの場合はこれらは全て1にする
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// 実際に頂点リソースを作る
	hr = dxCommon_->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&vertexResource_));
	assert(SUCCEEDED(hr));
}

/*-------------------- VertexBufferViewの作成 --------------------*/
void Triangle::CreateVertexBufferView() {
	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはVecter4を3頂点分
	vertexBufferView_.SizeInBytes = sizeof(Vecter4) * 3;
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(Vecter4);
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

/*-------------------- 描画 --------------------*/
void Triangle::Draw() {
	assert(dxCommon_ != nullptr);
	assert(trianglePSO_ != nullptr);
	assert(trianglePSO_->GetRootSignature() != nullptr);
	assert(trianglePSO_->GetPipelineState() != nullptr);

	// コマンドリストを取得(クラス化により必要に)
	auto commandList = dxCommon_->GetCommandList();
	commandList->RSSetViewports(1, &dxCommon_->GetViewPort());					// ViewPortを設定
	commandList->RSSetScissorRects(1, &dxCommon_->GetScissorRect());			// ScissorRectを設定
	// RootSignatureを設定。PSOとは別途設定する必要があることに注意
	commandList->SetGraphicsRootSignature(trianglePSO_->GetRootSignature());
	commandList->SetPipelineState(trianglePSO_->GetPipelineState());			// PSOを設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);					// VertexBufferViewを設定
	// 形状を設定。こちらもPSOとは別途設定する必要がある
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// 描画！(DrawCall)3頂点で一つのインスタンス
	commandList->DrawInstanced(3, 1, 0, 0);
}
#pragma once
#include <d3d12.h>
#include <cstdint>
#include"struct.h"

// 前方宣言
class DirectXCommon;
class TextureManager;
class WinApp;

class Sprite {
public:
	~Sprite();

	void Initialize(DirectXCommon* dxCommon, TextureManager* textureManager, WinApp* winApp);
	void Update();
	void Draw(uint32_t textureIndex);

private:
	// VertexResourceとVertexBufferViewの作成関数
	void CreateVertexResource();
	void CreateVertexBufferView();
	void MapVertexData();

	// 変換行列用のResourceとデータの作成関数
	void CreateTransformationMatrixResource();
	void MapTransformationMatrix();

	// ポインタを用意しておく
	DirectXCommon* dxCommon_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	WinApp* winApp_ = nullptr;

	// 図形の変換を管理するためのTransform
	Transform transform_;

	// スプライト用頂点構造体
	struct VertexData {
		Vector4 pos;
		Vector2 texcoord;
		Vector3 normal;
	};

	// 頂点リソースと頂点バッファビュー
	ID3D12Resource* vertexResource_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	VertexData* vertexData = nullptr;

	// 変換行列リソースと変換行列データ
	ID3D12Resource* transformationMatrixResource_ = nullptr;
	TransformationMatrix* transformationMatrixData_ = nullptr;
};
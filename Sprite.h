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
	void Draw();

	// ゲッター
	Vector3& GetUVTransformScale() { return uvTransform_.scale; }
	Vector3& GetUVTransformRotate() { return uvTransform_.rotate; }
	Vector3& GetUVTransformTranslate() { return uvTransform_.translate; }

private:
	// VertexResourceとVertexBufferViewの作成関数
	void CreateVertexResource();
	void CreateVertexBufferView();
	void MapVertexData();

	// IndexResourceとIndexBufferViewの作成関数
	void CreateIndexResource();
	void CreateIndexBufferView();
	void MapIndexData();

	// マテリアル用のResourceとデータの作成関数
	void CreateMaterialResource();
	void MapMaterial();

	// 変換行列用のResourceとデータの作成関数
	void CreateTransformationMatrixResource();
	void MapTransformationMatrix();

	// ポインタを用意しておく
	DirectXCommon* dxCommon_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	WinApp* winApp_ = nullptr;

	// テクスチャのインデックス
	uint32_t textureIndex_ = 0;

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

	// Index用のリソースとバッファビュー
	ID3D12Resource* indexResource_ = nullptr;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

	// マテリアルデータの構造体
	struct Material {
		Vector4 color;
		int32_t enableLighting;
		float padding[3];
		Matrix4x4 uvTransform;
	};
	// UVTransform用の変数
	Transform uvTransform_ = {
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f},
	};

	// マテリアルリソース
	ID3D12Resource* materialResource_ = nullptr;
	// マテリアルデータ
	Material* materialData_ = nullptr;

	// 変換行列リソースと変換行列データ
	ID3D12Resource* transformationMatrixResource_ = nullptr;
	TransformationMatrix* transformationMatrixData_ = nullptr;
};
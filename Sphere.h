#pragma once
#include <d3d12.h>
#include <cstdint>
#include"struct.h"
#include "Model.h"

// 前方宣言
class DirectXCommon;
class SpherePSO;
class TextureManager;
class DirectionalLight;
class Camera;

class Sphere {
public:
	~Sphere();

	void Initialize(DirectXCommon* dxCommon, SpherePSO* spherePSO, TextureManager* textureManager, DirectionalLight* directionalLight);
	void Update(const Camera& camera);
	void Draw(uint32_t textureIndex);

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
	SpherePSO* spherePSO_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	DirectionalLight* directionalLight_ = nullptr;
	Model* model_;

	// モデルデータ
	ModelData modelData_ = {};

	// 図形の変換を管理するためのTransformとワールド行列
	Transform transform_;
	Matrix4x4 worldMatrix_{};

	// 球用頂点構造体
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
	uint32_t* indexData_ = nullptr;
	uint32_t indexCount_ = 0;

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

	// 球の分割数
	static const uint32_t kSubdivision_ = 16;
	const uint32_t vertexCount_ = (kSubdivision_ + 1) * (kSubdivision_ + 1);
};
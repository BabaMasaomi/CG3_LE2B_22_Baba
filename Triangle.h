#pragma once
#include <d3d12.h>
#include"struct.h"

// 前方宣言
class DirectXCommon;
class TrianglePSO;
class Camera;

class Triangle {
public:
	~Triangle();

	void Initialize(DirectXCommon* dxCommon, TrianglePSO* trianglePSO);
	void Update(const Camera& camera);
	void Draw();

private:
	void CreateVertexResource();
	void CreateVertexBufferView();
	void MapVertexData();

	// マテリアル用のResourceとデータの作成関数
	void CreateMaterialResource();
	void MapMaterial();

	// 変換行列用のResourceとデータの作成関数
	void CreateTransformationMatrixResource();
	void MapTransformationMatrix();

	// DirectXCommonとTrianglePSOは、描画に必要な情報を持っているので、ポインタを用意しておく
	DirectXCommon* dxCommon_ = nullptr;
	TrianglePSO* trianglePSO_ = nullptr;

	// 頂点リソースと頂点バッファビュー
	ID3D12Resource* vertexResource_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	// マテリアルリソースとマテリアルデータ
	ID3D12Resource* materialResource_ = nullptr;
	Vector4* materialData_ = nullptr;

	// 変換行列リソースと変換行列データ
	ID3D12Resource* wvpResource_ = nullptr;
	TransformationMatrix* wvpMatrixData_ = nullptr;

	Transform transform_;

	// 頂点リソースにデータを書き込む
	Vector4* vertexData_ = nullptr;
};


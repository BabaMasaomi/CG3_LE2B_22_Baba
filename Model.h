#pragma once
#include <d3d12.h>
#include <cstdint>
#include "struct.h"
#include <string>
#include <vector>
#include <wrl.h>

// 前方宣言
class DirectXCommon;
class TextureManager;

struct VertexData {
	Vector4 pos;
	Vector2 texcoord;
	Vector3 normal;
};

struct MaterialData {
	std::string textureFilePath;
	Vector3 diffuseColor;
	Vector3 specularColor;
	float shininess;
};

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};

class Model {
public:
	void Initialize(DirectXCommon* dxCommon, TextureManager* textureManager);

	// objファイルを読み込む
	void LoadObjFile(const std::string& directryPath, const std::string& filename);

	// mtlファイルを読み込む
	MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	// ゲッター
	const ModelData& GetModelData() const { return modelData_; }
	const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const { return vertexBufferView_; }
	const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const { return indexBufferView_; }
	uint32_t GetIndexCount() const { return indexCount_; }

private:
	// VertexResourceとVertexBufferViewの作成関数
	void CreateVertexResource();
	void CreateVertexBufferView();
	void MapVertexData();

	// IndexResourceとIndexBufferViewの作成関数
	void CreateIndexResource();
	void CreateIndexBufferView();
	void MapIndexData();

	// ポインタを用意
	DirectXCommon* dxCommon_ = nullptr;
	TextureManager* textureManager_ = nullptr;

	// ModelDataを保持する
	ModelData modelData_ = {};

	// 頂点リソースと頂点バッファビュー
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	VertexData* vertexData = nullptr;

	// Index用のリソースとバッファビュー
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_ = nullptr;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};
	uint32_t* indexData_ = nullptr;
	uint32_t indexCount_ = 0;

	// 分割数
	static const uint32_t kSubdivision_ = 16;
	const uint32_t vertexCount_ = (kSubdivision_ + 1) * (kSubdivision_ + 1);
};
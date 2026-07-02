#pragma once
#include <d3d12.h>
#include <cstdint>
#include"struct.h"
#include "Model.h"
#include <wrl.h>

// 前方宣言
class DirectXCommon;
class ModelPSO;
class TextureManager;
class DirectionalLight;
class Camera;

class Object3d {
public:
	~Object3d();

	void Initialize(DirectXCommon* dxCommon, ModelPSO* modelPSO, TextureManager* textureManager, DirectionalLight* directionalLight);
	void Update(const Camera& camera);
	void Draw();

	// ゲッター
	Vector3& GetScale() { return scale_; }
	Vector3& GetRotate() { return rotate_; }
	Vector3& GetTranslate() { return translate_; }

	// セッター
	//void SetModel(Model* model) { model_ = model; }

private:
	// VertexResourceとVertexBufferViewの作成関数　消す
	void CreateVertexResource();
	void CreateVertexBufferView();
	void MapVertexData();

	// IndexResourceとIndexBufferViewの作成関数　消す
	void CreateIndexResource();
	void CreateIndexBufferView();
	void MapIndexData();

	// マテリアル用のResourceとデータの作成関数　消す
	void CreateMaterialResource();
	void MapMaterial();

	// 変換行列用のResourceとデータの作成関数
	void CreateTransformationMatrixResource();
	void MapTransformationMatrix();

	// ポインタを用意しておく
	DirectXCommon* dxCommon_ = nullptr;
	ModelPSO* modelPSO_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	DirectionalLight* directionalLight_ = nullptr;
	Model model_;

	// テクスチャのインデックス
	uint32_t textureIndex_ = 0;

	// 図形の変換を管理するためのTransformとワールド行列
	Transform transform_;		// 残す
	Matrix4x4 worldMatrix_{};	// 残す

	// 頂点構造体
	struct VertexData {
		Vector4 pos;
		Vector2 texcoord;
		Vector3 normal;
	};

	// 頂点リソースと頂点バッファビュー　多分消す
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	VertexData* vertexData = nullptr;

	// Index用のリソースとバッファビュー　多分消す
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_ = nullptr;
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

	// マテリアルリソース　多分消す
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;
	// マテリアルデータ
	Material* materialData_ = nullptr;

	// 変換行列リソースと変換行列データ
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_ = nullptr;	// 残す
	TransformationMatrix* transformationMatrixData_ = nullptr;

	// 分割数
	static const uint32_t kSubdivision_ = 16;
	const uint32_t vertexCount_ = (kSubdivision_ + 1) * (kSubdivision_ + 1);

	// ImGuiで弄る用
	Vector3 scale_;
	Vector3 rotate_;
	Vector3 translate_;
};
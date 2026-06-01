#pragma once
#include <d3d12.h>
#include <cstdint>
#include"struct.h"

// 前方宣言
class DirectXCommon;
class SpherePSO;
class TextureManager;
class Camera;

class Sphere {
public:
    ~Sphere();

    void Initialize(DirectXCommon* dxCommon,SpherePSO* spherePSO, TextureManager* textureManager);
    void Update(const Camera& camera);
    void Draw(uint32_t textureIndex);

private:
    // VertexResourceとVertexBufferViewの作成関数
    void CreateVertexResource();
    void CreateVertexBufferView();
    void MapVertexData();

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

    // 図形の変換を管理するためのTransformとワールド行列
    Transform transform_;
    Matrix4x4 worldMatrix_{};

    // スプライト用頂点構造体
    struct VertexData {
        Vector4 pos;
        Vector2 texcoord;
    };

    // 頂点リソースと頂点バッファビュー
    ID3D12Resource* vertexResource_ = nullptr;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
    VertexData* vertexData = nullptr;

    // マテリアルリソースとマテリアルデータ
    ID3D12Resource* materialResource_ = nullptr;
    Vector4* materialData_ = nullptr;


    // 変換行列リソースと変換行列データ
    ID3D12Resource* transformationMatrixResource_ = nullptr;
    TransformationMatrix* transformationMatrixData_ = nullptr;

    // 球の分割数
    static const uint32_t kSubdivision_ = 16;
    const uint32_t vertexCount_ = kSubdivision_ * kSubdivision_ * 6;
};
#pragma once
#include <d3d12.h>
#include <cstdint>
#include"struct.h"
#include <wrl.h>

namespace Engine {

	// 前方宣言
	class DirectXCommon;
	class ModelPSO;
	class TextureManager;
	class DirectionalLight;
	class Camera;

	class Sphere {
	public:
		/*==================== メンバ関数とデストラクタの宣言 ====================*/
		// デストラクタ
		~Sphere();

		void Initialize(DirectXCommon* dxCommon, ModelPSO* modelPSO, TextureManager* textureManager, DirectionalLight* light);

		void Update(const Camera& camera);

		void Draw(uint32_t textureIndex);

		/*==================== アクセッサ ====================*/
		// ゲッター
		Transform& GetTransform() { return transform_; }

	private:
		/*==================== メンバ関数 ====================*/
		// メッシュを作成する
		void CreateMesh();
		// VertexResourceの生成
		void CreateVertexResource();
		// VertexBufferViewの作成
		void CreateVertexBufferView();
		// VertexResourceにデータを書き込む
		void MapVertexData();
		// IndexResourceの生成
		void CreateIndexResource();
		// IndexBufferViewの作成
		void CreateIndexBufferView();
		// IndexResourceにデータを書き込む
		void MapIndexData();

		// マテリアルを作成する
		void CreateMaterial();
		// MaterialResourceの生成
		void CreateMaterialResource();
		// MaterialResourceにデータを書き込む
		void MapMaterial();

		// 変換行列を作成する
		void CreateTransform();
		// 変換行列用のResourceの生成
		void CreateTransformationMatrixResource();
		// 変換行列用のResourceにデータを書き込む
		void MapTransformationMatrix();

		/*==================== メンバ変数 ====================*/
		// ポインタを用意しておく
		DirectXCommon* dxCommon_ = nullptr;
		ModelPSO* modelPSO_ = nullptr;
		TextureManager* textureManager_ = nullptr;
		DirectionalLight* directionalLight_ = nullptr;

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
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
		VertexData* vertexData_ = nullptr;

		// Index用のリソースとバッファビュー
		Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
		D3D12_INDEX_BUFFER_VIEW indexBufferView_;
		uint32_t* indexData_ = nullptr;
		uint32_t indexCount_ = 0;

		// マテリアルデータの構造体
		struct Material {
			Vector4 color;
			int32_t enableLighting;
			float padding[3];
			Matrix4x4 uvTransform;
		};
		// マテリアルリソース
		Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
		// マテリアルデータ
		Material* materialData_ = nullptr;

		// 変換行列リソースと変換行列データ
		Microsoft::WRL::ComPtr<ID3D12Resource> transformResource_;
		TransformationMatrix* transformData_ = nullptr;

		// 球の分割数
		static const uint32_t kSubdivision_ = 16;
		const uint32_t vertexCount_ = (kSubdivision_ + 1) * (kSubdivision_ + 1);
	};
}
#pragma once
#include <d3d12.h>
#include <cstdint>
#include "struct.h"
#include <string>
#include <vector>
#include <wrl.h>

namespace Engine {

	// 前方宣言
	class DirectXCommon;
	class TextureManager;

	struct VertexData {
		Vector4 pos;
		Vector2 texcoord;
		Vector3 normal;
	};

	// マテリアルデータの構造体
	struct Material {
		Vector4 color;
		int32_t enableLighting;
		float padding[3];
		Matrix4x4 uvTransform;
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
		/*==================== メンバ関数の宣言 ====================*/
		/// <summary>
		/// モデルの初期化
		/// </summary>
		/// <param name="dxCommon">DirectXCommonのインスタンス</param>
		/// <param name="textureManager">TextureManagerのインスタンス</param>
		/// <param name="directory">ファイルディレクトリ</param>
		/// <param name="filename">ファイル名</param>
		void Initialize(DirectXCommon* dxCommon, TextureManager* textureManager,
			const std::string& directory, const std::string& filename);

		/// <summary>
		/// OBJファイルを読み込む
		/// </summary>
		/// <param name="directryPath">ファイルディレクトリ</param>
		/// <param name="filename">ファイル名</param>
		void LoadObjFile(const std::string& directryPath, const std::string& filename);

		/// <summary>
		/// マテリアルテンプレートファイルを読み込む
		/// </summary>
		/// <param name="directoryPath">ファイルディレクトリ</param>
		/// <param name="filename">ファイル名</param>
		/// <returns>読み込まれたマテリアルデータ</returns>
		MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

		/// <summary>
		/// UV変換行列を更新する
		/// </summary>
		/// <param name="uvTransform">更新するUV変換行列</param>
		void UpdateUVTransform(const Matrix4x4& uvTransform);

		/*==================== アクセッサ ====================*/
		// ゲッター
		D3D12_GPU_DESCRIPTOR_HANDLE GetTextureHandle() const;
		const ModelData& GetModelData() const { return modelData_; }
		const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const { return vertexBufferView_; }
		const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const { return indexBufferView_; }
		uint32_t GetIndexCount() const { return indexCount_; }
		ID3D12Resource* GetMaterialResource() const { return materialResource_.Get(); }

		// セッター
		void SetVertexBufferView(D3D12_VERTEX_BUFFER_VIEW vertexBufferView) { vertexBufferView_ = vertexBufferView; }
		void SetIndexBufferView(D3D12_INDEX_BUFFER_VIEW indexBufferView) { indexBufferView_ = indexBufferView; }

	private:
		/*==================== メンバ関数の宣言 ====================*/
		// VertexResourceとVertexBufferViewの作成関数	
		/// <summary>
		/// 頂点リソースを作成する
		/// </summary>
		void CreateVertexResource();
		/// <summary>
		/// 頂点バッファビューを作成する
		/// </summary>
		void CreateVertexBufferView();
		/// <summary>
		/// 頂点データをマップする
		/// </summary>
		void MapVertexData();

		// マテリアル用のResourceとデータの作成関数
		/// <summary>
		/// マテリアルリソースを作成する
		/// </summary>
		void CreateMaterialResource();
		/// <summary>
		/// マテリアルデータをマップする
		/// </summary>
		void MapMaterial();

		/*==================== メンバ変数 ====================*/
		// DirectXCommonのインスタンス
		DirectXCommon* dxCommon_ = nullptr;
		// TextureManagerのインスタンス
		TextureManager* textureManager_ = nullptr;

		// テクスチャのインデックス
		uint32_t textureIndex_ = 0;

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

		// マテリアルリソース
		Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;
		// マテリアルデータ
		Material* materialData_ = nullptr;
	};
}
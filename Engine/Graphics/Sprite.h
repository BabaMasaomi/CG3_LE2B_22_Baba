#pragma once
#include <d3d12.h>
#include <cstdint>
#include"struct.h"
#include <wrl.h>

namespace Engine {

	// 前方宣言
	class DirectXCommon;
	class SpritePSO;
	class TextureManager;
	class WinApp;

	class Sprite {
	public:
		/*==================== メンバ関数とデストラクタの宣言 ====================*/
		// デストラクタ
		~Sprite();

		/// <summary>
		/// Spriteの初期化
		/// </summary>
		/// <param name="dxCommon">DirectXCommonのインスタンス</param>
		/// <param name="spritePSO">SpritePSOのインスタンス</param>
		/// <param name="textureManager">TextureManagerのインスタンス</param>
		/// <param name="winApp">WinAppのインスタンス</param>
		void Initialize(DirectXCommon* dxCommon, SpritePSO* spritePSO, TextureManager* textureManager, WinApp* winApp);
		/// <summary>
		/// Spriteの更新
		/// </summary>
		void Update();
		/// <summary>
		/// Spriteの描画
		/// </summary>
		void Draw();

		/*==================== アクセッサ ====================*/
		// ゲッター
		const Vector3 GetUVTransformScale() { return uvTransform_.scale; }
		const Vector3 GetUVTransformRotate() { return uvTransform_.rotate; }
		const Vector3 GetUVTransformTranslate() { return uvTransform_.translate; }

		// セッター
		void SetUVTransformScale(Vector3 scale) { uvTransform_.scale = scale; }
		void SetUVTransformRotate(Vector3 rotate) { uvTransform_.rotate = rotate; }
		void SetUVTransformTranslate(Vector3 translate) { uvTransform_.translate = translate; }

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

		// IndexResourceとIndexBufferViewの作成関数
		/// <summary>
		/// Indexリソースを作成する
		/// </summary>
		void CreateIndexResource();
		/// <summary>
		/// Indexバッファビューを作成する
		/// </summary>
		void CreateIndexBufferView();
		/// <summary>
		/// Indexデータをマップする
		/// </summary>
		void MapIndexData();

		// マテリアル用のResourceとデータの作成関数
		/// <summary>
		/// マテリアルリソースを作成する
		/// </summary>
		void CreateMaterialResource();
		/// <summary>
		/// マテリアルデータをマップする
		/// </summary>
		void MapMaterial();

		// 変換行列用のResourceとデータの作成関数
		/// <summary>
		/// 変換行列用のリソースを作成する
		/// </summary>
		void CreateTransformationMatrixResource();
		/// <summary>
		/// 変換行列データをマップする
		/// </summary>
		void MapTransformationMatrix();

		/*==================== メンバ変数 ====================*/
		// DirectXCommonのインスタンス
		DirectXCommon* dxCommon_ = nullptr;
		// SpritePSOのインスタンス
		SpritePSO* spritePSO_ = nullptr;
		// TextureManagerのインスタンス
		TextureManager* textureManager_ = nullptr;
		// WinAppのインスタンス
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

		// 頂点リソース
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_ = nullptr;
		// 頂点バッファビュー
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
		// 頂点データ
		VertexData* vertexData = nullptr;

		// Indexリソース
		Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_ = nullptr;
		// Indexバッファビュー
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
			{1.0f,1.0f,1.0f},	// scale
			{0.0f,0.0f,0.0f},	// rotate
			{0.0f,0.0f,0.0f},	// translate
		};

		// マテリアルリソース
		Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;
		// マテリアルデータ
		Material* materialData_ = nullptr;

		// 変換行列リソース
		Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_ = nullptr;
		// 変換行列データ
		TransformationMatrix* transformationMatrixData_ = nullptr;
	};
}
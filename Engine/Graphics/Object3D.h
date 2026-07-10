#pragma once
#include <d3d12.h>
#include <cstdint>
#include"struct.h"
#include <wrl.h>

namespace Engine {

	// 前方宣言
	class DirectXCommon;
	class ModelPSO;
	class DirectionalLight;
	class Model;
	class Camera;

	class Object3D {
	public:
		/*==================== メンバ関数とデストラクタの宣言 ====================*/
		// デストラクタ
		~Object3D();

		/// <summary>
		/// Object3Dの初期化
		/// </summary>
		/// <param name="dxCommon">DirectXCommonのインスタンス</param>
		/// <param name="modelPSO">ModelPSOのインスタンス</param>
		/// <param name="directionalLight">DirectionalLightのインスタンス</param>
		void Initialize(DirectXCommon* dxCommon, ModelPSO* modelPSO, DirectionalLight* directionalLight);
		/// <summary>
		/// Object3Dの更新
		/// </summary>
		/// <param name="camera">カメラのインスタンス</param>
		void Update(const Camera& camera);
		/// <summary>
		/// Object3Dの描画
		/// </summary>
		void Draw();

		/// <summary>
		/// モデルを設定
		/// </summary>
		/// <param name="model">設定するモデルのインスタンス</param>
		void SetModel(Model* model);

		/*==================== アクセッサ ====================*/
		// ゲッター
		Vector3& GetScale() { return scale_; }
		Vector3& GetRotate() { return rotate_; }
		Vector3& GetTranslate() { return translate_; }

		// セッター
		void SetScale(Vector3 scale) { scale_ = scale; }
		void SetRotate(Vector3 rotate) { rotate_ = rotate; }
		void SetTranslate(Vector3 translate) { translate_ = translate; }

	private:
		/*==================== メンバ関数の宣言 ====================*/
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
		// ModelPSOのインスタンス
		ModelPSO* modelPSO_ = nullptr;
		// DirectionalLightのインスタンス
		DirectionalLight* directionalLight_ = nullptr;
		// モデルのインスタンス
		Model* model_ = nullptr;

		// テクスチャのインデックス
		uint32_t textureIndex_ = 0;

		// 図形の変換を管理するためのTransformとワールド行列
		Transform transform_;
		Matrix4x4 worldMatrix_{};

		// 頂点構造体
		struct VertexData {
			Vector4 pos;
			Vector2 texcoord;
			Vector3 normal;
		};

		// UVTransform用の変数
		Transform uvTransform_ = {
			{1.0f,1.0f,1.0f},
			{0.0f,0.0f,0.0f},
			{0.0f,0.0f,0.0f},
		};

		// 変換行列用のリソース
		Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_ = nullptr;
		// 変換行列データ
		TransformationMatrix* transformationMatrixData_ = nullptr;

		// ImGuiで弄る用
		Vector3 scale_;
		Vector3 rotate_;
		Vector3 translate_;
	};
}
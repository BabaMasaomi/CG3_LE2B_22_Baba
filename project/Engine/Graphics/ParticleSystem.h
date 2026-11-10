#pragma once
#include <d3d12.h>
#include <cstdint>
#include <wrl.h>
#include <array>
#include"struct.h"

namespace Engine {

	// 前方宣言
	class DirectXCommon;
	class ParticlePSO;
	class DirectionalLight;
	class Model;
	class Camera;

	// ParticleSystemごとのライティング設定
	struct ObjectLightingData {
		int32_t lightingType;
		float padding[3];
	};

	class ParticleSystem {
	public:
		/*==================== メンバ関数とデストラクタの宣言 ====================*/
		// デストラクタ
		~ParticleSystem();

		/// <summary>
		/// Particleの初期化
		/// </summary>
		/// <param name="dxCommon">DirectXCommonのインスタンス</param>
		/// <param name="modelPSO">ParticlePSOのインスタンス</param>
		/// <param name="directionalLight">DirectionalLightのインスタンス</param>
		void Initialize(DirectXCommon* dxCommon, ParticlePSO* particlePSO, DirectionalLight* directionalLight);
		/// <summary>
		/// Particleの更新
		/// </summary>
		/// <param name="camera">カメラのインスタンス</param>
		void Update(const Camera& camera);
		/// <summary>
		/// Particleの描画
		/// </summary>
		void Draw();

		/// <summary>
		/// モデルを設定
		/// </summary>
		/// <param name="model">設定するモデルのインスタンス</param>
		void SetModel(Model* model);

		/*==================== アクセッサ ====================*/
		// ゲッター
		const Vector3 GetScale() { return scale_; }
		const Vector3 GetRotate() { return rotate_; }
		const Vector3 GetTranslate() { return translate_; }
		const Vector4& GetColor() const { return color_; }
		const Transform& GetUVTransform() const { return uvTransform_; }
		LightingMode GetLightingMode() const { return lightingMode_; }

		// セッター
		void SetScale(Vector3 scale) { scale_ = scale; }
		void SetRotate(Vector3 rotate) { rotate_ = rotate; }
		void SetTranslate(Vector3 translate) { translate_ = translate; }
		void SetColor(const Vector4& color) { color_ = color; }
		void SetUVTransform(const Transform& uvTransform) { uvTransform_ = uvTransform; }
		void SetLightingMode(LightingMode lightingMode) { lightingMode_ = lightingMode; }

		void SetInstanceCount(uint32_t count);
		void SetInstanceTransform(uint32_t index, const Transform& transform);

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

		/// <summary>
		/// Particle用にInstancing用の変換行列バッファを生成・初期化する
		/// </summary>
		void CreateInstancingResource();

		/// <summary>
		/// SRV作成処理
		/// </summary>
		void CreateInstancingSRV();

		/// <summary>
		/// マテリアル用のResourceとデータの作成関数
		/// </summary>
		void CreateMaterialResource();
		/// <summary>
		/// マテリアルを更新する
		/// </summary>
		void UpdateMaterial();

		/*==================== メンバ変数 ====================*/
		// DirectXCommonのインスタンス
		DirectXCommon* dxCommon_ = nullptr;
		// ParticlePSOのインスタンス
		ParticlePSO* particlePSO_ = nullptr;
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

		// 変換行列用のリソース
		Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_ = nullptr;
		// 変換行列データ
		TransformationMatrix* transformationMatrixData_ = nullptr;

		// Particle用の変数群
		// 1回の描画で扱える最大インスタンス数
		static constexpr uint32_t kMaxInstanceCount = 10;
		// Instancing用の変換行列バッファ
		Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;
		// Mapして取得した、配列の先頭アドレス
		TransformationMatrix* instancingData_ = nullptr;
		// 
		D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU_{};

		// 各インスタンスの相対的な配置
		std::array<Transform, kMaxInstanceCount> instanceTransforms_{};
		// 実際に描画する個数。最初は既存表示を保つため1個
		uint32_t instanceCount_ = 1;

		// ImGuiで弄る用
		Vector3 scale_;
		Vector3 rotate_;
		Vector3 translate_;

		// Particleごとのマテリアル
		Microsoft::WRL::ComPtr<ID3D12Resource>materialResource_ = nullptr;
		// Particleごとのマテリアルデータ
		Material* materialData_ = nullptr;
		// Particleごとの色
		Vector4 color_ = { 1.0f,1.0f,1.0f,1.0f };
		// UVTransform用の変数
		Transform uvTransform_ = {
			{1.0f,1.0f,1.0f},	// scale
			{0.0f,0.0f,0.0f},	// rotate
			{0.0f,0.0f,0.0f},	// translate
		};

		// Particleごとのライティング形式
		LightingMode lightingMode_ = LightingMode::HalfLambert;
		// Particleごとのライティング設定用のリソース
		Microsoft::WRL::ComPtr<ID3D12Resource>objectLightingResource_ = nullptr;
		// Particleごとのライティング設定用のデータ
		ObjectLightingData* objectLightingData_ = nullptr;
	};
}
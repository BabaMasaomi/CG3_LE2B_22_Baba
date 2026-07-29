#pragma once
#include <d3d12.h>
#include <wrl.h>
#include"struct.h"

namespace Engine {

	// ライティングモード
	enum class LightingMode {
		None,
		Lambert,
		HalfLambert,
	};

	struct DirectionalLightData {
		Vector4 color;			// ライトの色
		Vector3 direction;		// ライトの向き
		float intensity;		// 輝度
		int lightingMode;	// ライティングモード
		float padding[3];		// パディング
	};

	class DirectXCommon;

	class DirectionalLight {
	public:
		/*==================== メンバ関数とデストラクタの宣言 ====================*/
		// デストラクタ
		~DirectionalLight();

		/// <summary>
		/// DirectionalLightの初期化
		/// </summary>
		/// <param name="dxCommon">DirectXCommonのインスタンス</param>
		void Initialize(DirectXCommon* dxCommon);
		/// <summary>
		/// DirectionalLightの更新
		/// </summary>
		void Update();

		/*==================== アクセッサ ====================*/
		// ゲッター
		ID3D12Resource* GetResource() const { return resource_.Get(); }
		const Vector4 GetColor() { return color_; }
		const Vector3 GetDirection() { return direction_; }
		const float GetIntensity() { return intensity_; }
		LightingMode GetLightingMode() const { return lightingMode_; }

		// セッター
		void SetColor(Vector4 color) { color_ = color; }
		void SetDirection(Vector3 direction) { direction_ = direction; }
		void SetIntensity(float intensity) { intensity_ = intensity; }
		void SetLightingMode(LightingMode mode) { lightingMode_ = mode; }

	private:
		/*==================== メンバ変数 ====================*/
		// DirectXCommonのインスタンス
		DirectXCommon* dxCommon_ = nullptr;

		// DirectionalLightDataのリソース
		Microsoft::WRL::ComPtr<ID3D12Resource> resource_ = nullptr;
		// DirectionalLightDataのポインタ
		DirectionalLightData* data_ = nullptr;

		// ImGuiで弄る用
		Vector4 color_ = { 1.0f,1.0f,1.0f,1.0f };
		Vector3 direction_ = { 0.0f,-1.0f,0.0f };
		float intensity_ = 1.0f;

		// ライティングモード
		LightingMode lightingMode_ = LightingMode::HalfLambert;

	};
}
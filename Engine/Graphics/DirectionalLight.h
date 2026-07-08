#pragma once
#include <d3d12.h>
#include <wrl.h>
#include"struct.h"

struct DirectionalLightData {
	Vector4 color;      // ライトの色
	Vector3 direction;  // ライトの向き
	float intensity;    // 輝度
};

class DirectXCommon;

class DirectionalLight {
public:
	~DirectionalLight();

	void Initialize(DirectXCommon* dxCommon);
	void Update();

	// ゲッター
	ID3D12Resource* GetResource() const { return resource_.Get(); }
	Vector4& GetColor() { return color_; }
	Vector3& GetDirection() { return direction_; }
	float& GetIntensity() { return intensity_; }

private:
	// ポインタを用意
	DirectXCommon* dxCommon_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> resource_ = nullptr;
	DirectionalLightData* data_ = nullptr;

	// ImGuiで弄る用
	Vector4 color_ = { 1.0f,1.0f,1.0f,1.0f };
	Vector3 direction_ = { 0.0f,-1.0f,0.0f };
	float intensity_ = 1.0f;

};
#include "DirectionalLight.h"
#include "DirectXCommon.h"
#include "Utility.h"

namespace Engine {

	// デストラクタ
	DirectionalLight::~DirectionalLight() {
		OutputDebugStringA("~DirectionalLight\n");
	}

	/*-------------------- 初期化 --------------------*/
	void DirectionalLight::Initialize(DirectXCommon* dxCommon) {
		dxCommon_ = dxCommon;

		// 定数バッファ生成
		resource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(DirectionalLightData));

		// Map
		resource_->Map(0, nullptr, reinterpret_cast<void**>(&data_));

		data_->color = { 1.0f,1.0f,1.0f,1.0f };
		data_->direction = { 0.0f,-1.0f,0.0f };
		data_->intensity = 1.0f;
	}

	/*-------------------- 更新 --------------------*/
	void DirectionalLight::Update() {
		// LightのSRVを変更する
		data_->color = color_;
		data_->direction = direction_;
		data_->intensity = intensity_;
	}
}
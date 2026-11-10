#include "ParticleSystem.h"
#include "DirectXCommon.h"
#include "ParticlePSO.h"
#include "Model.h"
#include "DirectionalLight.h"
#include "Camera.h"
#include "Function.h"
#include "Logger.h"
#include <format>			// フォーマットの自動指定をしてくれる(+安全策)
#include <cmath>
#include <cassert>
#include <cstring>
#include <stdexcept>

namespace Engine {

	// デストラクタ
	ParticleSystem::~ParticleSystem() {
		OutputDebugStringA("~ParticleSystem\n");
	}

	/*-------------------- 初期化 --------------------*/
	void ParticleSystem::Initialize(DirectXCommon* dxCommon, ParticlePSO* particlePSO, DirectionalLight* directionalLight) {

		// ポインタを保存しておく
		dxCommon_ = dxCommon;
		particlePSO_ = particlePSO;
		directionalLight_ = directionalLight;

		// CPUで動かす用のTransformを作る
		transform_ = { {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };

		// 10枚動かす用のTransform
		for (uint32_t i = 0; i < kMaxInstanceCount; ++i) {
			instanceTransforms_[i] = {
				{1.0f, 1.0f, 1.0f}, // scale
				{0.0f, 0.0f, 0.0f}, // rotate
				{0.0f, 0.0f, 0.0f}  // translate
			};
		}

		// ImGui用
		scale_ = transform_.scale;
		rotate_ = transform_.rotate;
		translate_ = transform_.translate;

		// 変換行列用のResourceとデータの作成関数
		CreateTransformationMatrixResource();
		MapTransformationMatrix();

		// マテリアル用のResourceとデータの作成関数
		CreateMaterialResource();

		// Object3Dごとのライティング設定用リソース
		objectLightingResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(ObjectLightingData));

		// Particle用にリソースを作成
		CreateInstancingResource();
		CreateInstancingSRV();

		// 書き込むためのアドレスを取得
		HRESULT hr = objectLightingResource_->Map(0, nullptr, reinterpret_cast<void**>(&objectLightingData_));
		assert(SUCCEEDED(hr));

		// ライティングモードを設定
		objectLightingData_->lightingType = static_cast<int32_t>(lightingMode_);
		// パディングを設定
		objectLightingData_->padding[0] = 0.0f;
		objectLightingData_->padding[1] = 0.0f;
		objectLightingData_->padding[2] = 0.0f;
	}

	/*-------------------- 変換行列用のResourceの生成 --------------------*/
	void ParticleSystem::CreateTransformationMatrixResource() {
		transformationMatrixResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(TransformationMatrix));
	}

	/*-------------------- 変換行列用のResourceにデータを書き込む --------------------*/
	void ParticleSystem::MapTransformationMatrix() {
		// 書き込むためのアドレスを取得
		HRESULT hr = transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
		assert(SUCCEEDED(hr));
		// 単位行列を書きこんでおく
		transformationMatrixData_->WVP = MakeIdentity4x4();
		transformationMatrixData_->World = MakeIdentity4x4();
	}

	/*-------------------- Particle用にInstancing用の変換行列バッファを生成・初期化する --------------------*/
	void ParticleSystem::CreateInstancingResource() {
		// TransformationMatrixを10個格納する領域を確保
		instancingResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(TransformationMatrix) * kMaxInstanceCount);
		// CPUから書き込めるようにする
		HRESULT hr = instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));

		assert(SUCCEEDED(hr));

		// 全要素を単位行列で初期化
		for (uint32_t i = 0; i < kMaxInstanceCount; ++i) {
			instancingData_[i].WVP = MakeIdentity4x4();
			instancingData_[i].World = MakeIdentity4x4();
		}
	}

	/*-------------------- SRV作成処理 --------------------*/
	void ParticleSystem::CreateInstancingSRV() {
		// このObject3D専用の配置番号を確保
		const uint32_t srvIndex = dxCommon_->AllocateInstancingSrv();

		auto* heap = dxCommon_->GetSrvDescriptorHeap();
		const uint32_t descriptorSize = dxCommon_->GetDescriptorSizeSRV();

		const auto cpuHandle = GetCPUDescriptorHandle(heap, descriptorSize, srvIndex);

		instancingSrvHandleGPU_ = GetGPUDescriptorHandle(heap, descriptorSize, srvIndex);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Shader4ComponentMapping =
			D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;

		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = kMaxInstanceCount;
		srvDesc.Buffer.StructureByteStride = sizeof(TransformationMatrix);
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		dxCommon_->GetDevice()->CreateShaderResourceView(instancingResource_.Get(), &srvDesc, cpuHandle);
	}

	/*-------------------- マテリアル用のResourceとデータの作成関数 --------------------*/
	void ParticleSystem::CreateMaterialResource() {
		// マテリアル用のResouceを作成
		materialResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(Material));

		HRESULT hr = materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
		assert(SUCCEEDED(hr));

		// 初期値を設定
		materialData_->color = color_;
		materialData_->enableLighting = true;

		materialData_->padding[0] = 0.0f;
		materialData_->padding[1] = 0.0f;
		materialData_->padding[2] = 0.0f;

		materialData_->uvTransform = MakeIdentity4x4();
	}

	/*-------------------- マテリアルを更新する --------------------*/
	void ParticleSystem::UpdateMaterial() {
		// マテリアルの色を更新
		materialData_->color = color_;
		// マテリアルのUVTransformを更新
		materialData_->uvTransform = MakeAffineMatrix(
			uvTransform_.scale,
			uvTransform_.rotate,
			uvTransform_.translate
		);
	}

	/*-------------------- 更新 --------------------*/
	void ParticleSystem::Update(const Camera& camera) {
		// UVTransform用の行列を作成
		Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransform_.scale);
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransform_.rotate.z));
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransform_.translate));
		model_->UpdateUVTransform(uvTransformMatrix);

		// ImGuiの変更を反映する
		transform_.scale = scale_;
		transform_.rotate = rotate_;
		transform_.translate = translate_;

		// ワールド行列を作成
		worldMatrix_ = MakeAffineMatrix(
			transform_.scale,
			transform_.rotate,
			transform_.translate
		);

		// WVP行列を計算して、CBufferに書き込む
		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix_, camera.GetViewProjectionMatrix());

		for (uint32_t i = 0; i < instanceCount_; ++i) {
			const Transform& instance = instanceTransforms_[i];

			const Matrix4x4 localMatrix = MakeAffineMatrix(
				instance.scale,
				instance.rotate,
				instance.translate);

			// 各インスタンスの配置に、Object3D全体の変換を適用
			const Matrix4x4 instanceWorld = Multiply(localMatrix, worldMatrix_);

			instancingData_[i].World = instanceWorld;
			instancingData_[i].WVP = Multiply(instanceWorld, camera.GetViewProjectionMatrix());
		}

		// マテリアルを更新
		UpdateMaterial();

		// ライティングモードを設定
		objectLightingData_->lightingType = static_cast<int32_t>(lightingMode_);
	}

	/*-------------------- 描画 --------------------*/
	void ParticleSystem::Draw() {
		// コマンドリストを取得
		auto* commandList = dxCommon_->GetCommandList();
		// RootSignature設定
		commandList->SetGraphicsRootSignature(particlePSO_->GetRootSignature());
		commandList->SetPipelineState(particlePSO_->GetPipelineState());			// PSOを設定
		commandList->IASetVertexBuffers(0, 1, &model_->GetVertexBufferView());	// VertexBufferViewを設定
		// 形状を設定
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());	// マテリアルCBufferの場所を設定
		commandList->SetGraphicsRootDescriptorTable(1, instancingSrvHandleGPU_);						// wvp用のCBufferの場所を設定
		// SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である
		commandList->SetGraphicsRootDescriptorTable(2, model_->GetTextureHandle());
		commandList->SetGraphicsRootConstantBufferView(3, directionalLight_->GetResource()->GetGPUVirtualAddress());	// 平行光源を適用
		commandList->SetGraphicsRootConstantBufferView(4, objectLightingResource_->GetGPUVirtualAddress());				// Object3Dごとのライティング設定
		// 描画！(DrawCall)
		commandList->DrawInstanced(UINT(model_->GetModelData().vertices.size()), instanceCount_, 0, 0);
	}

	/*-------------------- モデルを設定 --------------------*/
	void ParticleSystem::SetModel(Model* model) {
		// モデルを設定
		model_ = model;

		// モデルのマテリアルの拡散色を取得して、Object3Dのマテリアルに反映
		const Vector3& diffuseColor = model_->GetModelData().material.diffuseColor;
		// 拡散色を反映
		color_ = { diffuseColor.x,diffuseColor.y,diffuseColor.z,1.0f };
		// マテリアルデータが存在する場合は、マテリアルデータの色も更新
		if (materialData_) {
			materialData_->color = color_;
		}
	}

	/*-------------------- セッター --------------------*/
	void ParticleSystem::SetInstanceCount(uint32_t count) {
		if (count > kMaxInstanceCount) {
			throw std::out_of_range("インスタンス数が上限を超えています");
		}

		instanceCount_ = count;
	}

	void ParticleSystem::SetInstanceTransform(
		uint32_t index, const Transform& transform) {

		if (index >= kMaxInstanceCount) {
			throw std::out_of_range("インスタンス番号が範囲外です");
		}

		instanceTransforms_[index] = transform;
	}
}
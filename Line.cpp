#include "Line.h"

#include "Camera.h"
#include "DirectXCommon.h"
#include "LinePSO.h"
#include "Utility.h"

#include <cassert>

namespace Engine {

	/*-------------------- 初期化 --------------------*/
	void Line::Initialize(
		DirectXCommon* dxCommon,
		LinePSO* linePSO,
		const Vector3& start,
		const Vector3& end) {

		assert(dxCommon != nullptr);
		assert(linePSO != nullptr);

		dxCommon_ = dxCommon;
		linePSO_ = linePSO;

		start_ = start;
		end_ = end;

		// 頂点リソース
		CreateVertexResource();

		// 変換行列リソース
		CreateTransformationResource();

		// 色用リソース
		CreateMaterialResource();

		// 初期値を反映
		UpdateVertexData();

		transformationData_->WVP =
			MakeIdentity4x4();

		// TransformationMatrixにWorldがある場合
		transformationData_->World =
			MakeIdentity4x4();

		materialData_->color = color_;
	}

	/*-------------------- 頂点リソースの作成 --------------------*/
	void Line::CreateVertexResource() {
		// 始点と終点の2頂点
		vertexResource_ =
			CreateBufferResource(
				dxCommon_->GetDevice(),
				sizeof(VertexData) * 2
			);

		// 頂点バッファビュー
		vertexBufferView_.BufferLocation =
			vertexResource_->GetGPUVirtualAddress();

		vertexBufferView_.SizeInBytes =
			sizeof(VertexData) * 2;

		vertexBufferView_.StrideInBytes =
			sizeof(VertexData);

		// 頂点データをMap
		HRESULT result =
			vertexResource_->Map(
				0,
				nullptr,
				reinterpret_cast<void**>(
					&vertexData_
					)
			);

		assert(SUCCEEDED(result));
	}

	/*-------------------- 頂点データの更新 --------------------*/
	void Line::UpdateVertexData() {
		// 始点
		vertexData_[0].position = {
			start_.x,
			start_.y,
			start_.z,
			1.0f
		};

		// 終点
		vertexData_[1].position = {
			end_.x,
			end_.y,
			end_.z,
			1.0f
		};
	}

	/*-------------------- 変換行列リソースの作成 --------------------*/
	void Line::CreateTransformationResource() {
		transformationResource_ =
			CreateBufferResource(
				dxCommon_->GetDevice(),
				sizeof(TransformationMatrix)
			);

		HRESULT result =
			transformationResource_->Map(
				0,
				nullptr,
				reinterpret_cast<void**>(
					&transformationData_
					)
			);

		assert(SUCCEEDED(result));
	}

	/*-------------------- 色用リソースの作成 --------------------*/
	void Line::CreateMaterialResource() {
		materialResource_ =
			CreateBufferResource(
				dxCommon_->GetDevice(),
				sizeof(LineMaterial)
			);

		HRESULT result =
			materialResource_->Map(
				0,
				nullptr,
				reinterpret_cast<void**>(
					&materialData_
					)
			);

		assert(SUCCEEDED(result));
	}

	/*-------------------- 更新 --------------------*/
	void Line::Update(const Camera& camera) {
		// SetStart、SetEndで変更された座標を反映
		UpdateVertexData();

		// 今回は始点・終点をワールド座標として扱う
		Matrix4x4 worldMatrix =
			MakeIdentity4x4();

		Matrix4x4 viewProjectionMatrix =
			camera.GetViewProjectionMatrix();

		transformationData_->WVP =
			Multiply(
				worldMatrix,
				viewProjectionMatrix
			);

		// TransformationMatrixにWorldがある場合
		transformationData_->World =
			worldMatrix;

		// 色を反映
		materialData_->color =
			color_;
	}

	/*-------------------- 描画 --------------------*/
	void Line::Draw() {
		ID3D12GraphicsCommandList* commandList =
			dxCommon_->GetCommandList();

		// RootSignature
		commandList->SetGraphicsRootSignature(
			linePSO_->GetRootSignature()
		);

		// PipelineState
		commandList->SetPipelineState(
			linePSO_->GetPipelineState()
		);

		// 線分として描画
		commandList->IASetPrimitiveTopology(
			D3D_PRIMITIVE_TOPOLOGY_LINELIST
		);

		// 頂点バッファを設定
		commandList->IASetVertexBuffers(
			0,
			1,
			&vertexBufferView_
		);

		// 色の定数バッファ
		commandList->
			SetGraphicsRootConstantBufferView(
				0,
				materialResource_->
				GetGPUVirtualAddress()
			);

		// WVPの定数バッファ
		commandList->
			SetGraphicsRootConstantBufferView(
				1,
				transformationResource_->
				GetGPUVirtualAddress()
			);

		// 始点と終点の2頂点を描画
		commandList->DrawInstanced(
			2,
			1,
			0,
			0
		);
	}

}
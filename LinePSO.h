#pragma once

#include <d3d12.h>
#include <dxcapi.h>
#include <wrl.h>

namespace Engine {

	class DirectXCommon;
	class DXCManager;

	class LinePSO {
	public:
		/// <summary>
		/// 線分用PSOの初期化
		/// </summary>
		void Initialize(
			DirectXCommon* dxCommon, DXCManager* dxcManager
		);

		/*==================== アクセッサ ====================*/
		ID3D12RootSignature*
			GetRootSignature() const {

			return rootSignature_.Get();
		}

		ID3D12PipelineState*
			GetPipelineState() const {

			return pipelineState_.Get();
		}

	private:
		/*==================== メンバ関数の宣言 ====================*/
		// RootSignatureとGraphicsPipelineの生成関数
		/// <summary>
		/// RootSignatureの生成
		/// </summary>
		void CreateRootSignature();
		/// <summary>
		/// GraphicsPipelineの生成
		/// </summary>
		void CreateGraphicsPipeline();

		DirectXCommon* dxCommon_ = nullptr;
		// DXCManagerのインスタンス
		DXCManager* dxcManager_ = nullptr;

		Microsoft::WRL::ComPtr<ID3D12RootSignature>
			rootSignature_ = nullptr;

		Microsoft::WRL::ComPtr<ID3D12PipelineState>
			pipelineState_ = nullptr;

		// Shader Blob
		Microsoft::WRL::ComPtr<IDxcBlob>
			vertexShaderBlob_ = nullptr;

		Microsoft::WRL::ComPtr<IDxcBlob>
			pixelShaderBlob_ = nullptr;
	};

}
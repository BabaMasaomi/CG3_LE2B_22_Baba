#include "LinePSO.h"

#include "DirectXCommon.h"
#include "DXCManager.h"
#include "Utility.h"

#include <cassert>
#include <string>

namespace Engine {

	/*-------------------- 初期化 --------------------*/
	void LinePSO::Initialize(DirectXCommon* dxCommon, DXCManager* dxcManager) {
		assert(dxCommon != nullptr);

		dxCommon_ = dxCommon;
		dxcManager_ = dxcManager;

		// Shaderをコンパイル
		vertexShaderBlob_ = dxcManager_->CompileShader(
			L"Line.VS.hlsl",
			L"vs_6_0",
			dxcManager_->GetDxcUtils(),
			dxcManager_->GetDxcCompiler(),
			dxcManager_->GetIncludeHandler()
		);

		assert(vertexShaderBlob_ != nullptr);

		pixelShaderBlob_ = dxcManager_->CompileShader(
			L"Line.PS.hlsl",
			L"ps_6_0",
			dxcManager_->GetDxcUtils(),
			dxcManager_->GetDxcCompiler(),
			dxcManager_->GetIncludeHandler()
		);

		assert(pixelShaderBlob_ != nullptr);

		// RootSignatureを作成
		CreateRootSignature();

		// GraphicsPipelineStateを作成
		CreateGraphicsPipeline();
	}

	/*-------------------- RootSignatureの作成 --------------------*/
	void LinePSO::CreateRootSignature() {
		// RootParameter
		D3D12_ROOT_PARAMETER rootParameters[2]{};

		// rootParameter[0]
		// LineMaterial : b0 / Pixel Shader
		rootParameters[0].ParameterType =
			D3D12_ROOT_PARAMETER_TYPE_CBV;

		rootParameters[0].ShaderVisibility =
			D3D12_SHADER_VISIBILITY_PIXEL;

		rootParameters[0].Descriptor.ShaderRegister =
			0;

		rootParameters[0].Descriptor.RegisterSpace =
			0;

		// rootParameter[1]
		// TransformationMatrix : b1 / Vertex Shader
		rootParameters[1].ParameterType =
			D3D12_ROOT_PARAMETER_TYPE_CBV;

		rootParameters[1].ShaderVisibility =
			D3D12_SHADER_VISIBILITY_VERTEX;

		rootParameters[1].Descriptor.ShaderRegister =
			1;

		rootParameters[1].Descriptor.RegisterSpace =
			0;

		// RootSignatureの設定
		D3D12_ROOT_SIGNATURE_DESC
			rootSignatureDesc{};

		rootSignatureDesc.Flags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		rootSignatureDesc.pParameters =
			rootParameters;

		rootSignatureDesc.NumParameters =
			_countof(rootParameters);

		rootSignatureDesc.pStaticSamplers =
			nullptr;

		rootSignatureDesc.NumStaticSamplers =
			0;

		// Serialize
		Microsoft::WRL::ComPtr<ID3DBlob>
			signatureBlob = nullptr;

		Microsoft::WRL::ComPtr<ID3DBlob>
			errorBlob = nullptr;

		HRESULT result =
			D3D12SerializeRootSignature(
				&rootSignatureDesc,
				D3D_ROOT_SIGNATURE_VERSION_1,
				signatureBlob.GetAddressOf(),
				errorBlob.GetAddressOf()
			);

		if (FAILED(result)) {
			if (errorBlob != nullptr) {
				OutputDebugStringA(
					reinterpret_cast<const char*>(
						errorBlob->GetBufferPointer()
						)
				);
			}

			assert(false);
		}

		// RootSignatureを生成
		result =
			dxCommon_->GetDevice()->
			CreateRootSignature(
				0,
				signatureBlob->GetBufferPointer(),
				signatureBlob->GetBufferSize(),
				IID_PPV_ARGS(
					rootSignature_.GetAddressOf()
				)
			);

		assert(SUCCEEDED(result));
	}

	/*-------------------- GraphicsPipelineStateの作成 --------------------*/
	void LinePSO::CreateGraphicsPipeline() {
		//==============================
		// InputLayout
		//==============================

		D3D12_INPUT_ELEMENT_DESC
			inputElementDescs[1]{};

		inputElementDescs[0].SemanticName =
			"POSITION";

		inputElementDescs[0].SemanticIndex =
			0;

		inputElementDescs[0].Format =
			DXGI_FORMAT_R32G32B32A32_FLOAT;

		inputElementDescs[0].InputSlot =
			0;

		inputElementDescs[0].AlignedByteOffset =
			D3D12_APPEND_ALIGNED_ELEMENT;

		inputElementDescs[0].InputSlotClass =
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

		inputElementDescs[0].InstanceDataStepRate =
			0;

		D3D12_INPUT_LAYOUT_DESC
			inputLayoutDesc{};

		inputLayoutDesc.pInputElementDescs =
			inputElementDescs;

		inputLayoutDesc.NumElements =
			_countof(inputElementDescs);

		//==============================
		// BlendState
		//==============================

		D3D12_BLEND_DESC blendDesc{};

		blendDesc.RenderTarget[0].BlendEnable =
			TRUE;

		blendDesc.RenderTarget[0].LogicOpEnable =
			FALSE;

		blendDesc.RenderTarget[0].SrcBlend =
			D3D12_BLEND_SRC_ALPHA;

		blendDesc.RenderTarget[0].DestBlend =
			D3D12_BLEND_INV_SRC_ALPHA;

		blendDesc.RenderTarget[0].BlendOp =
			D3D12_BLEND_OP_ADD;

		blendDesc.RenderTarget[0].SrcBlendAlpha =
			D3D12_BLEND_ONE;

		blendDesc.RenderTarget[0].DestBlendAlpha =
			D3D12_BLEND_ZERO;

		blendDesc.RenderTarget[0].BlendOpAlpha =
			D3D12_BLEND_OP_ADD;

		blendDesc.RenderTarget[0].LogicOp =
			D3D12_LOGIC_OP_NOOP;

		blendDesc.RenderTarget[0].RenderTargetWriteMask =
			D3D12_COLOR_WRITE_ENABLE_ALL;

		//==============================
		// RasterizerState
		//==============================

		D3D12_RASTERIZER_DESC rasterizerDesc{};

		// 線分なのでカリング不要
		rasterizerDesc.CullMode =
			D3D12_CULL_MODE_NONE;

		rasterizerDesc.FillMode =
			D3D12_FILL_MODE_SOLID;

		rasterizerDesc.DepthClipEnable =
			TRUE;

		//==============================
		// DepthStencilState
		//==============================

		D3D12_DEPTH_STENCIL_DESC
			depthStencilDesc{};

		// 3D空間上の線なので深度テストを有効
		depthStencilDesc.DepthEnable =
			TRUE;

		// 通常の線は深度を書き込む
		depthStencilDesc.DepthWriteMask =
			D3D12_DEPTH_WRITE_MASK_ALL;

		depthStencilDesc.DepthFunc =
			D3D12_COMPARISON_FUNC_LESS_EQUAL;

		depthStencilDesc.StencilEnable =
			FALSE;

		//==============================
		// GraphicsPipelineStateDesc
		//==============================

		D3D12_GRAPHICS_PIPELINE_STATE_DESC
			graphicsPipelineStateDesc{};

		graphicsPipelineStateDesc.pRootSignature =
			rootSignature_.Get();

		graphicsPipelineStateDesc.InputLayout =
			inputLayoutDesc;

		graphicsPipelineStateDesc.VS = {
			vertexShaderBlob_->GetBufferPointer(),
			vertexShaderBlob_->GetBufferSize()
		};

		graphicsPipelineStateDesc.PS = {
			pixelShaderBlob_->GetBufferPointer(),
			pixelShaderBlob_->GetBufferSize()
		};

		graphicsPipelineStateDesc.BlendState =
			blendDesc;

		graphicsPipelineStateDesc.RasterizerState =
			rasterizerDesc;

		graphicsPipelineStateDesc.DepthStencilState =
			depthStencilDesc;

		graphicsPipelineStateDesc.DSVFormat =
			DXGI_FORMAT_D24_UNORM_S8_UINT;

		graphicsPipelineStateDesc.NumRenderTargets =
			1;

		graphicsPipelineStateDesc.RTVFormats[0] =
			DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		// 線分用PSOなのでLINE
		graphicsPipelineStateDesc.PrimitiveTopologyType =
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

		graphicsPipelineStateDesc.SampleDesc.Count =
			1;

		graphicsPipelineStateDesc.SampleMask =
			D3D12_DEFAULT_SAMPLE_MASK;

		// PSOを生成
		HRESULT result =
			dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&pipelineState_));

		assert(SUCCEEDED(result));
	}

}
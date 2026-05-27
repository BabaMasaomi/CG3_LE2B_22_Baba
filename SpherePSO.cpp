#include "SpherePSO.h"
#include "DirectXCommon.h"
#include "DXCManager.h"
#include "Logger.h"
#include <format>			// フォーマットの自動指定をしてくれる(+安全策)
#include <cassert>

SpherePSO::~SpherePSO() {
	if (pipelineState_) pipelineState_->Release();
	if (rootSignature_) rootSignature_->Release();
}

/*-------------------- 初期化 --------------------*/
void SpherePSO::Initialize(DirectXCommon* dxCommon, DXCManager* dxcManager) {
	dxCommon_ = dxCommon;
	dxcManager_ = dxcManager;
	CreateRootSignature();
	CreateGraphicsPipeline();
}

/* -------------------- - グラフィックスパイプラインの生成-------------------- */
void SpherePSO::CreateGraphicsPipeline() {
	HRESULT hr;

	// InputLayoutの設定
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	// 全ての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// Shaderのコンパイル
	// vertexShader
	IDxcBlob* vertexShaderBlob = dxcManager_->CompileShader(L"Object3D.VS.hlsl",
		L"vs_6_0", dxcManager_->GetDxcUtils(), dxcManager_->GetDxcCompiler(), dxcManager_->GetIncludeHandler()
	);
	assert(vertexShaderBlob != nullptr);

	// pixelShader
	IDxcBlob* pixelShaderBlob = dxcManager_->CompileShader(L"Object3D.PS.hlsl",
		L"ps_6_0", dxcManager_->GetDxcUtils(), dxcManager_->GetDxcCompiler(), dxcManager_->GetIncludeHandler()
	);
	assert(pixelShaderBlob != nullptr);

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	// Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	//書き込みする
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	// 比較関数はLessEqual (近ければ描画される)
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// PSOの生成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature_;													// RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;													// InputLayout
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };	// VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };	// PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;															// BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;													// RasterizerState
	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ(形状)のタイプ。今回は三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// 深度ステンシルの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// どの様に画面に打ち込むかの設定(気にせんで良い)
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// 実際に生成
	hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&pipelineState_));
	assert(SUCCEEDED(hr));

	// 解放
	if (vertexShaderBlob) vertexShaderBlob->Release();
	if (pixelShaderBlob) pixelShaderBlob->Release();
}

// -------------------- - ルートシグネチャの生成-------------------- * //	
void SpherePSO::CreateRootSignature() {
	HRESULT hr;

	// RootSignatureの設定
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// RootParameterを作成
	D3D12_ROOT_PARAMETER rootParameters[3] = {};
	// WVP用
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;		// CBVを使う(b1のbと一致)
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;	// VertexShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0;						// レジスタ番号1を使う

	// Material用
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;		// CBVを使う(b0のbと一致)
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;		// PixelShaderで使う
	rootParameters[1].Descriptor.ShaderRegister = 1;						// レジスタ番号0とバインド(b0の0と一致)
	
	descriptionRootSignature.pParameters = rootParameters;					// ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);		// 配列の長さ
	// DescriptorRangeを用意(SRVのため)
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;														// 0から始まる
	descriptorRange[0].NumDescriptors = 1;															// 数は1つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;									// SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;	// Offsetは自動計算
	// SRV用
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;					// DescriptorTableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;				// PixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;				// Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);	// Tableで利用する数

	// Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// シリアライズしてバイナリにする
	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリをもとに作成
	hr = dxCommon_->GetDevice()->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));
	// 解放
	if (signatureBlob) signatureBlob->Release();
	if (errorBlob) errorBlob->Release();
}
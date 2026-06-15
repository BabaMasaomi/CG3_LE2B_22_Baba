#pragma once
#include <d3d12.h>
#include <wrl.h>

class DirectXCommon;
class DXCManager;

class ModelPSO {
public:
	~ModelPSO();

	void Initialize(DirectXCommon* dxCommon, DXCManager* dxcManager);

	// ゲッター
	ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }
	ID3D12PipelineState* GetPipelineState() const { return pipelineState_.Get(); }

private:
	// RootSignatureとGraphicsPipelineの生成関数
	void CreateRootSignature();
	void CreateGraphicsPipeline();

	// DirectXCommonとDXCManagerは、PSOの生成に必要な情報を持っているので、ポインタを用意しておく
	DirectXCommon* dxCommon_ = nullptr;
	DXCManager* dxcManager_ = nullptr;

	// PSOを生成するために必要なRootSignatureとPipelineStateを用意しておく
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_ = nullptr;
};
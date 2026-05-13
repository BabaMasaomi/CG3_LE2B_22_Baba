#pragma once
#include <d3d12.h>

class DirectXCommon;
class DXCManager;

class TrianglePSO {
public:
	~TrianglePSO();

	void Initialize(
		DirectXCommon* dxCommon,
		DXCManager* dxcManager
	);

	// ゲッター
	ID3D12RootSignature* GetRootSignature() const {return rootSignature_;}
	ID3D12PipelineState* GetPipelineState() const {return pipelineState_;}

private:
	void CreateRootSignature();
	void CreateGraphicsPipeline();

	DirectXCommon* dxCommon_ = nullptr;
	DXCManager* dxcManager_ = nullptr;

	ID3D12RootSignature* rootSignature_ = nullptr;
	ID3D12PipelineState* pipelineState_ = nullptr;
};
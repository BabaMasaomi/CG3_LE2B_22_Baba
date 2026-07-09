#pragma once
#include <d3d12.h>
#include <wrl.h>

class DirectXCommon;
class DXCManager;

class ModelPSO {
public:
	// デストラクタ
	~ModelPSO();

	/// <summary>
	/// ModelPSOの初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのインスタンス</param>
	/// <param name="dxcManager">DXCManagerのインスタンス</param>
	void Initialize(DirectXCommon* dxCommon, DXCManager* dxcManager);

	// ゲッター
	ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }
	ID3D12PipelineState* GetPipelineState() const { return pipelineState_.Get(); }
	ID3D12PipelineState* GetSpritePipelineState() { return spritePipelineState_.Get(); }

private:
	// RootSignatureとGraphicsPipelineの生成関数
	/// <summary>
	/// RootSignatureの生成
	/// </summary>
	void CreateRootSignature();
	/// <summary>
	/// GraphicsPipelineの生成
	/// </summary>
	void CreateGraphicsPipeline();
	/// <summary>
	/// SpritePipelineの生成
	/// </summary>
	void CreateSpritePipeline();

	// DirectXCommonのインスタンス
	DirectXCommon* dxCommon_ = nullptr;
	// DXCManagerのインスタンス
	DXCManager* dxcManager_ = nullptr;

	// PSOを生成するために必要なRootSignatureとPipelineStateを用意しておく
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> spritePipelineState_ = nullptr;
};
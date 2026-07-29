#pragma once
#include <d3d12.h>
#include <wrl.h>

namespace Engine {

	// 前方宣言
	class DirectXCommon;
	class DXCManager;

	class SpritePSO {
	public:
		/*==================== メンバ関数とデストラクタの宣言 ====================*/
		// デストラクタ
		~SpritePSO();

		/// <summary>
		/// ModelPSOの初期化
		/// </summary>
		/// <param name="dxCommon">DirectXCommonのインスタンス</param>
		/// <param name="dxcManager">DXCManagerのインスタンス</param>
		void Initialize(DirectXCommon* dxCommon, DXCManager* dxcManager);

		/*==================== アクセッサ ====================*/
		// ゲッター
		ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }
		ID3D12PipelineState* GetSpritePipelineState() { return spritePipelineState_.Get(); }

	private:
		/*==================== メンバ関数の宣言 ====================*/
		// RootSignatureとGraphicsPipelineの生成関数
		/// <summary>
		/// RootSignatureの生成
		/// </summary>
		void CreateRootSignature();
		/// <summary>
		/// SpritePipelineの生成
		/// </summary>
		void CreateSpritePipeline();

		/*==================== メンバ変数 ====================*/
		// DirectXCommonのインスタンス
		DirectXCommon* dxCommon_ = nullptr;
		// DXCManagerのインスタンス
		DXCManager* dxcManager_ = nullptr;

		// PSOを生成するために必要なRootSignatureとPipelineStateを用意しておく
		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> spritePipelineState_ = nullptr;
	};
}
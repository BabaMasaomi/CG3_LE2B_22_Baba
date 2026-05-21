#pragma once
#include "externals/DirectXTex/DirectXTex.h"
#include <string>
#include <d3d12.h>

class DirectXCommon;

class TextureManager {
public:

	~TextureManager();

	void Initialize(DirectXCommon* dxCommon);

	// 1.Textureデータを読み込む(後でuint32_tを返す設計にしよう)
	DirectX::ScratchImage LoadTexture(const std::string& filePath);

	// 2.DirectX12のTextureResourceを作る
	ID3D12Resource* CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata);

	// 3.CPUで書きこむ用にUploadHeapのResourceを作る
	// 4.UploadHeapのResourceに対してCPUでデータを書きこむ
	// 5.CommandListにUploadHeapのResource(3.)をTextureResource(2.)に転送するコマンドを積む
	ID3D12Resource* UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages, ID3D12Device* device,
		ID3D12GraphicsCommandList* commandList);

	// ShaderResourceViewを作る
	void CreateSRV(ID3D12Resource* textureResource, uint32_t srvIndex);

	// ゲッター
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSrvHandleGPU()const { return textureSrvHandleGPU_; }

private:
	DirectXCommon* dxCommon_ = nullptr;

	// TextureResource
	ID3D12Resource* textureResource_ = nullptr;

	// Meta情報を取得
	DirectX::TexMetadata metaData_{};

	// 中間リソースを作成
	ID3D12Resource* intermediateResource_ = nullptr;

	// SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU_;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_;
};
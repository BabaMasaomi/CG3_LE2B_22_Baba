#pragma once
#include "externals/DirectXTex/DirectXTex.h"
#include <string>
#include <d3d12.h>
#include <wrl.h>

class DirectXCommon;

class TextureManager {
public:
	// デストラクタ
	~TextureManager();

	// テクスチャの最大数
	static const uint32_t kTextureCount = 256;

	/// <summary>
	/// TextureManagerの初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのインスタンス</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// テクスチャを読み込む
	/// </summary>
	/// <param name="filePath">ファイルパス</param>
	/// <returns>テクスチャのインデックス</returns>
	uint32_t LoadTexture(const std::string& filePath);

	/// <summary>
	/// TextureResourceを作成する
	/// </summary>
	/// <param name="device">DirectX12デバイス</param>
	/// <param name="metadata">テクスチャのメタデータ</param>
	/// <returns>作成されたTextureResource</returns>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata);

	/// <summary>
	/// TextureResourceにデータを転送する
	/// </summary>
	/// <param name="texture">TextureResource</param>
	/// <param name="mipImages">ミップマップ画像</param>
	/// <param name="device">DirectX12デバイス</param>
	/// <param name="commandList">CommandList</param>
	/// <returns>アップロードされたTextureResource</returns>
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages, ID3D12Device* device,
		ID3D12GraphicsCommandList* commandList);

	/// <summary>
	/// ShaderResourceViewを作成する
	/// </summary>
	/// <param name="textureResource">TextureResource</param>
	/// <param name="index">テクスチャのインデックス</param>
	void CreateSRV(ID3D12Resource* textureResource, uint32_t index);

	// ゲッター
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSrvHandleGPU(uint32_t index)const { return textureSrvHandleGPU_[index]; }
	uint32_t GetNextIndex() { return nextTextureIndex_; }
private:
	// DirectXCommonのインスタンス
	DirectXCommon* dxCommon_ = nullptr;

	// TextureResource
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResources_[kTextureCount] = {};
	// TextureResourceにデータを転送するための中間バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResources_[kTextureCount] = {};

	// Meta情報を取得
	DirectX::TexMetadata metaData_[kTextureCount]{};

	// SRVを作成するDescriptorHeapの場所を決める
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_[kTextureCount]{};

	// 次のテクスチャ番号を記録
	uint32_t nextTextureIndex_ = 1;
	// 読み込み済みのテクスチャのファイルパスとインデックスを記録するマップ
	std::unordered_map<std::string, uint32_t> textureMap_;
};
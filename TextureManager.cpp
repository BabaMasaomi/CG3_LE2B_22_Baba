#include "TextureManager.h"
#include "DirectXCommon.h"
#include "Utility.h"
#include <cassert>

void TextureManager::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;
	
}

/*-------------------- 1.Textureデータを読み込む --------------------*/	// mainなどでtextureManager.LoadTexture("resources/uvChecker.png");として呼び出す
DirectX::ScratchImage TextureManager::LoadTexture(const std::string& filePath) {
	// 1.テクスチャファイルを読み込んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// 2.mipMap作成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	// 3.metaDataを取得する
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	// 4.resource生成
	ID3D12Resource* textureResource = CreateTextureResource(dxCommon_->GetDevice(), metadata);

	// 5.転送
	UploadTextureData(textureResource, mipImages);

	// 6. SRV生成
	CreateSRV(textureResource, 1);

	// ミップマップ付きデータを返す
	return mipImages;
}

/*-------------------- 2.DirectX12のTextureResourceを作る --------------------*/
ID3D12Resource* TextureManager::CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata) {
	// 1.metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width);								// Textureの幅
	resourceDesc.Height = UINT(metadata.height);							// Textureの高さ
	resourceDesc.MipLevels = UINT16(metadata.mipLevels);					// mipmapの数
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);				// 奥行き or 配列Textureの配列数
	resourceDesc.Format = metadata.format;									// TextureのFormat
	resourceDesc.SampleDesc.Count = 1;										// サンプリングカウント。1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);	// Textureの次元数。普段使っているのは2次元

	// 2.利用するHeapの設定(非常に特殊な運用。02_04exで一般的な運用例がある)
	D3D12_HEAP_PROPERTIES heapPropaties{};
	heapPropaties.Type = D3D12_HEAP_TYPE_CUSTOM;							// 細かい設定を行う
	heapPropaties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;		// WriteBackポリシーでCPUアクセス可能
	heapPropaties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;				// プロセッサの近くに配置

	// 3.Resourceを生成する
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapPropaties,														// Heap設定
		D3D12_HEAP_FLAG_NONE,												// Heapの特殊な設定。特に無し
		&resourceDesc,														// Resourceの設定
		D3D12_RESOURCE_STATE_GENERIC_READ,									// 初回のResourceState。Textureは基本読むだけ
		nullptr,															// Clear最適値。使わないのでnullptr
		IID_PPV_ARGS(&resource));											// 作成するResourceポインタへのポインタ
	assert(SUCCEEDED(hr));
	return resource;
}

/*-------------------- 3.TextureResourceにデータを転送する --------------------*/
void TextureManager::UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages) {
	// Meta情報を取得
	metaData_ = mipImages.GetMetadata();
	// 全MipMapについて
	for (size_t mipLevel = 0; mipLevel < metaData_.mipLevels; ++mipLevel) {
		// MipMapLevelを指定して各Imageを取得
		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
		// Textureに転送
		HRESULT hr = texture->WriteToSubresource(
			UINT(mipLevel),
			nullptr,					// 全領域へコピー
			img->pixels,				// 元データアドレス
			UINT(img->rowPitch),		// 1ラインサイズ
			UINT(img->slicePitch));		// 1枚サイズ
		assert(SUCCEEDED(hr));
	}
}

/*-------------------- ShaderResourceViewを作る --------------------*/
void TextureManager::CreateSRV(ID3D12Resource* textureResource, uint32_t srvIndex) {
	// metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metaData_.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;						// 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metaData_.mipLevels);

	// SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = dxCommon_->GetsrvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = dxCommon_->GetsrvDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();
	// 先頭はImGuiが使っているのでその次を使う
	textureSrvHandleCPU.ptr += dxCommon_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU.ptr += dxCommon_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	// SRVの生成
	dxCommon_->GetDevice()->CreateShaderResourceView(textureResource, &srvDesc, textureSrvHandleCPU);
}
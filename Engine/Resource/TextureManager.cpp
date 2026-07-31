#include "externals/DirectXTex/d3dx12.h"
#include "TextureManager.h"
#include "DirectXCommon.h"
#include "Utility.h"
#include <cassert>
#include <vector>

namespace Engine {

	// デストラクタ
	TextureManager::~TextureManager() {
		OutputDebugStringA("~TextureManager\n");
	}

	/*-------------------- 初期化 --------------------*/
	void TextureManager::Initialize(DirectXCommon* dxCommon) {
		dxCommon_ = dxCommon;
	}

	/*-------------------- 1.Textureデータを読み込む --------------------*/	// mainなどでtextureManager.LoadTexture("uvChecker.png");として呼び出す
	uint32_t TextureManager::LoadTexture(const std::string& filePath) {
		// 読み込み済みならその番号を返す
		auto it = textureMap_.find(filePath);
		if (it != textureMap_.end()) {
			return it->second;
		}
		// 0.テクスチャインデックスを進める
		uint32_t index = nextTextureIndex_++;
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
		metaData_[index - 1] = mipImages.GetMetadata();
		//const DirectX::TexMetadata& metadata = metaData_;

		// 4.resource生成
		textureResources_[index - 1] = CreateTextureResource(dxCommon_->GetDevice(), metaData_[index - 1]);

		// 5.転送
		intermediateResources_[index - 1] = UploadTextureData(textureResources_[index - 1].Get(), mipImages, dxCommon_->GetDevice(), dxCommon_->GetCommandList());

		// 6.SRV生成
		CreateSRV(textureResources_[index - 1].Get(), index);

		// 7.番号を返す
		// 読み込みに成功したので番号を登録
		textureMap_[filePath] = index;
		return index;
	}

	/*-------------------- 2.DirectX12のTextureResourceを作る --------------------*/
	Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata) {
		// 1.metadataを基にResourceの設定
		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Width = UINT(metadata.width);								// Textureの幅
		resourceDesc.Height = UINT(metadata.height);							// Textureの高さ
		resourceDesc.MipLevels = UINT16(metadata.mipLevels);					// mipmapの数
		resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);				// 奥行き or 配列Textureの配列数
		resourceDesc.Format = metadata.format;									// TextureのFormat
		resourceDesc.SampleDesc.Count = 1;										// サンプリングカウント。1固定
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);	// Textureの次元数。普段使っているのは2次元

		// 2.利用するHeapの設定
		D3D12_HEAP_PROPERTIES heapPropaties{};
		heapPropaties.Type = D3D12_HEAP_TYPE_DEFAULT;							// 細かい設定を行う

		// 3.Resourceを生成する
		Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
		HRESULT hr = device->CreateCommittedResource(
			&heapPropaties,														// Heap設定
			D3D12_HEAP_FLAG_NONE,												// Heapの特殊な設定。特に無し
			&resourceDesc,														// Resourceの設定
			D3D12_RESOURCE_STATE_COPY_DEST,										// データ転送される設定
			nullptr,															// Clear最適値。使わないのでnullptr
			IID_PPV_ARGS(&resource));											// 作成するResourceポインタへのポインタ
		assert(SUCCEEDED(hr));

		return resource;
	}

	/*-------------------- 3.4.5.を一括で行う --------------------*/
	[[nodiscard]]
	Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages, ID3D12Device* device,
		ID3D12GraphicsCommandList* commandList) {
		// 3.CPUで書きこむ用にUploadHeapのResourceを作る
		std::vector<D3D12_SUBRESOURCE_DATA> subresource;
		DirectX::PrepareUpload(device, mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresource);
		uint64_t intermediateSize = GetRequiredIntermediateSize(texture, 0, UINT(subresource.size()));
		Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = CreateBufferResource(device, intermediateSize);

		// 4.UploadHeapのResourceに対してCPUでデータを書きこむ
		UpdateSubresources(commandList, texture, intermediateResource.Get(), 0, 0, UINT(subresource.size()), subresource.data());

		// 5.CommandListにUploadHeapのResource(3.)をTextureResource(2.)に転送するコマンドを積む
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = texture;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
		commandList->ResourceBarrier(1, &barrier);
		return intermediateResource;
	}

	/*-------------------- ShaderResourceViewを作る --------------------*/
	void TextureManager::CreateSRV(ID3D12Resource* textureResource, uint32_t index) {
		assert(index > 0);
		// metaDataを基にSRVの設定
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = metaData_[index - 1].format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;						// 2Dテクスチャ
		srvDesc.Texture2D.MipLevels = UINT(metaData_[index - 1].mipLevels);

		// SRVを作成するDescriptorHeapの場所を決める
		// 先頭はImGuiが使っているのでその次を使う
		auto cpuHandle = GetCPUDescriptorHandle(dxCommon_->GetSrvDescriptorHeap(), dxCommon_->GetDescriptorSizeSRV(), index);
		textureSrvHandleGPU_[index - 1] = GetGPUDescriptorHandle(dxCommon_->GetSrvDescriptorHeap(), dxCommon_->GetDescriptorSizeSRV(), index);
		// SRVの生成
		dxCommon_->GetDevice()->CreateShaderResourceView(textureResource, &srvDesc, cpuHandle);
	}
}
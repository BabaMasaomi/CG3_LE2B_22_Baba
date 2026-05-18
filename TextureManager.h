#pragma once
#include "externals/DirectXTex/DirectXTex.h"
#include <string>
#include <d3d12.h>

class DirectXCommon;

class TextureManager {
public:
    void Initialize(DirectXCommon* dxCommon);

    // 1.Textureデータを読み込む(後でuint32_tを返す設計にしよう)
    DirectX::ScratchImage LoadTexture(const std::string& filePath);

    // 2.DirectX12のTextureResourceを作る
    ID3D12Resource* CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata);

    // 3.TextureResourceにデータを転送する
    void UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages);

    // ShaderResourceViewを作る
    void CreateSRV(ID3D12Resource* textureResource,uint32_t srvIndex);

private:
    DirectXCommon* dxCommon_ = nullptr;

    // Meta情報を取得
    DirectX::TexMetadata metaData_{};
};
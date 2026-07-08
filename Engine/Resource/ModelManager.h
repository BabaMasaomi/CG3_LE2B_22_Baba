#pragma once
#include <memory>
#include <string>
#include <unordered_map>

// 前方宣言
class DirectXCommon;
class TextureManager;
class Model;

class ModelManager {
public:
	void Initialize(DirectXCommon* dxCommon, TextureManager* textureManager);

	// モデル読み込み
	Model* LoadModel(const std::string& directory, const std::string& filename);

private:
	// ポインタを用意しておく
	DirectXCommon* dxCommon_ = nullptr;
	TextureManager* textureManager_ = nullptr;

	// モデルのマップ
	std::unordered_map<std::string, std::unique_ptr<Model>> models_;

};


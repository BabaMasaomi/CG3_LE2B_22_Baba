#include "ModelManager.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "Model.h"

/*-------------------- 初期化 --------------------*/
void ModelManager::Initialize(DirectXCommon* dxCommon, TextureManager* textureManager) {
	dxCommon_ = dxCommon;
	textureManager_ = textureManager;
}

/*-------------------- モデル読み込み --------------------*/
Model* ModelManager::LoadModel(const std::string& directory, const std::string& filename) {
	// キーを設定
	std::string key = directory + "/" + filename;

	// 既に読み込んだか確認
	auto it = models_.find(key);
	if (it != models_.end()) {
		return it->second.get();
	}
	// 初めて読むので生成
	auto model = std::make_unique<Model>();

	// 初期化
	model->Initialize(dxCommon_, textureManager_, directory, filename);
	// 生成したモデルのポインタを返す
	Model* result = model.get();
	// マップに登録
	models_[key] = std::move(model);

	return result;
}
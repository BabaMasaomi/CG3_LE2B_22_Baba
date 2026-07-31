#include "ModelManager.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "Model.h"
#include "Sphere.h"

namespace Engine {

	/*-------------------- 初期化 --------------------*/
	void ModelManager::Initialize(DirectXCommon* dxCommon, TextureManager* textureManager) {
		dxCommon_ = dxCommon;
		textureManager_ = textureManager;
	}

	/*-------------------- モデル読み込み --------------------*/
	Model* ModelManager::LoadModel(const std::string& filename) {
		// キーを設定
		std::string key = "resources/Textures/" + filename;

		// 既に読み込んだか確認
		auto it = models_.find(key);
		if (it != models_.end()) {
			return it->second.get();
		}
		// 初めて読むので生成
		auto model = std::make_unique<Model>();

		// 初期化
		model->Initialize(dxCommon_, textureManager_, filename);
		// 生成したモデルのポインタを返す
		Model* result = model.get();
		// マップに登録
		models_[key] = std::move(model);

		return result;
	}

	/*-------------------- 球モデルを作成 --------------------*/
	Model* ModelManager::CreateSphereModel(const std::string& modelName, float radius,
		uint32_t subdivision, const std::string& textureFilePath) {
		// キーを設定
		const std::string key = "Primitive/Sphere/" + modelName;
		// 既に生成済みなら共有する
		auto it = models_.find(key);
		if (it != models_.end()) {
			return it->second.get();
		}
		// 生成済みでなければ新規作成
		ModelData sphereData = Sphere::Generate(radius, subdivision, textureFilePath);
		// Modelの生成
		auto model = std::make_unique<Model>();
		// 初期化
		model->Initialize(dxCommon_, textureManager_, std::move(sphereData));
		// 生成したモデルのポインタを返す
		Model* result = model.get();
		// マップに登録
		models_.emplace(key, std::move(model));

		return result;
	}
}
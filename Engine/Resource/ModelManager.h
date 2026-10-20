#pragma once
#include <memory>
#include <string>
#include <unordered_map>

namespace Engine {

	// 前方宣言
	class DirectXCommon;
	class TextureManager;
	class Model;

	class ModelManager {
	public:
		/*==================== メンバ関数の宣言 ====================*/
		/// <summary>
		/// ModelManagerの初期化
		/// </summary>
		/// <param name="dxCommon">DirectXCommonのインスタンス</param>
		/// <param name="textureManager">TextureManagerのインスタンス</param>
		void Initialize(DirectXCommon* dxCommon, TextureManager* textureManager);

		/// <summary>
		/// モデルを読み込む
		/// </summary>
		/// <param name="filename">ファイル名</param>
		/// <returns>読み込まれたモデルのポインタ</returns>
		Model* LoadModel(const std::string& filename);

		/// <summary>
		/// 球モデルを作成
		/// </summary>
		/// <param name="modelName">モデル名</param>
		/// <param name="radius">球の半径</param>
		/// <param name="subdivision">分割数</param>
		/// <param name="textureFilePath">使用するテクスチャ</param>
		/// <returns>管理クラスが所有する共有モデルへのポインタ</returns>
		Model* CreateSphereModel(const std::string& modelName, float radius,
			uint32_t subdivision, const std::string& textureFilePath);

	private:
		/*==================== メンバ変数 ====================*/
		// DirectXCommonのインスタンス
		DirectXCommon* dxCommon_ = nullptr;
		// TextureManagerのインスタンス
		TextureManager* textureManager_ = nullptr;

		// モデルのマップ
		std::unordered_map<std::string, std::unique_ptr<Model>> models_;

	};
}
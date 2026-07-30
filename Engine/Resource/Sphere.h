#pragma once

#include "Model.h"
#include <cstdint>
#include <string>

namespace Engine {

	/// <summary>
	/// 球体のモデルを生成するクラス
	/// </summary>
	class Sphere {
	public:
		/// <summary>
		/// 球体のModelDataを生成
		/// </summary>
		/// <param name="radius">半径</param>
		/// <param name="subdivision">縦横の分割数</param>
		/// <param name="textureFilePath">使用するテクスチャ</param>
		static ModelData Generate(float radius, uint32_t subdivision, const std::string& textureFilePath);
	};

}
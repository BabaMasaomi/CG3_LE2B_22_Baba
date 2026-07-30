#include "Sphere.h"

#include <cassert>
#include <cmath>

namespace Engine {

	/*-------------------- 球体のModelDataを生成 --------------------*/
	ModelData Sphere::Generate(
		float radius,
		uint32_t subdivision,
		const std::string& textureFilePath) {
		// 引数のチェック
		assert(radius > 0.0f);
		assert(subdivision >= 3);

		// ModelDataの生成
		ModelData modelData{};
		// Sphere用のマテリアルを設定
		modelData.material.textureFilePath = textureFilePath;

		// 1マスにつき三角形2枚、1三角形につき3頂点
		modelData.vertices.reserve(
			subdivision * subdivision * 6
		);

		// 球の頂点を生成
		const float pi = std::acos(-1.0f);
		const float lonEvery = 2.0f * pi / static_cast<float>(subdivision);
		const float latEvery = pi / static_cast<float>(subdivision);

		// ラムダ式で頂点を生成する関数を定義
		auto createVertex = [radius, subdivision, pi, lonEvery, latEvery](uint32_t lat, uint32_t lon) {
			// lat: 緯度、lon: 経度
			VertexData vertex{};
			// 緯度と経度をラジアンに変換
			const float latitude = -pi * 0.5f + latEvery * static_cast<float>(lat);
			const float longitude = lonEvery * static_cast<float>(lon);
			// 球の法線ベクトルを計算
			const Vector3 normal = {
				std::cos(latitude) * std::cos(longitude),
				std::sin(latitude),
				std::cos(latitude) * std::sin(longitude)
			};
			// 頂点の位置を計算
			vertex.pos = {
				normal.x * radius,
				normal.y * radius,
				normal.z * radius,
				1.0f
			};
			// UV座標を計算
			vertex.texcoord = {
				static_cast<float>(lon) / static_cast<float>(subdivision),
				1.0f - static_cast<float>(lat) / static_cast<float>(subdivision)
			};
			// 法線ベクトルを設定
			vertex.normal = normal;

			return vertex;
			};

		// 球の頂点を生成
		for (uint32_t lat = 0; lat < subdivision; ++lat) {
			for (uint32_t lon = 0; lon < subdivision; ++lon) {

				// 四角形の4頂点
				const VertexData current = createVertex(lat, lon);
				const VertexData next = createVertex(lat, lon + 1);
				const VertexData down = createVertex(lat + 1, lon);
				const VertexData downNext = createVertex(lat + 1, lon + 1);

				// 1枚目
				modelData.vertices.push_back(current);
				modelData.vertices.push_back(down);
				modelData.vertices.push_back(next);

				// 2枚目
				modelData.vertices.push_back(next);
				modelData.vertices.push_back(down);
				modelData.vertices.push_back(downNext);
			}
		}

		return modelData;
	}

}
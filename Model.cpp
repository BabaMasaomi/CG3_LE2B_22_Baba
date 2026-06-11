#include "Model.h"
#include <fstream>
#include <sstream>
#include <cassert>

/*-------------------- objファイルを読み込む --------------------*/
ModelData Model::LoadObjFile(const std::string& directryPath, const std::string& filename) {
	// 1.中で必要となる変数の宣言
	ModelData modelData;			// 構築するModelData
	std::vector<Vector4> positions;	// 位置
	std::vector<Vector3> normals;	// 法線
	std::vector<Vector2> texcoords;	// テクスチャ座標
	std::string line;				// ファイルから読んだ1行を格納するもの

	// 2.ファイルを開く
	std::ifstream file(directryPath + "/" + filename);	// ファイルを開く
	assert(file.is_open());	// 開けなかったら止める

	// 3.実際にファイルを読み、ModelDataを構築していく
	while (std::getline(file, line)) {
		std::string identfier;
		std::istringstream s(line);
		s >> identfier;	// 先頭の識別子を読む

		// identfierに応じた処理
		// 頂点情報を読む
		if (identfier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			positions.push_back(position);

		} else if (identfier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoords.push_back(texcoord);

		} else if (identfier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);

		} else if (identfier == "f") {
			// 三角形を作る
			// 面は三角形限定、その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				// 頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');	// 区切りでインデックスを読んでいく
					elementIndices[element] = std::stoi(index);
				}
				// 各要素のIndexから、実際の要素の値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				VertexData vertex = { position,texcoord,normal };
				modelData.vertices.push_back(vertex);
			}
		}
	}

	// 4.ModelDataを返す
	return modelData;
}
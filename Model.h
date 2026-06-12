#pragma once
#include "struct.h"
#include <string>
#include <vector>

struct VertexData {
	Vector4 pos;
	Vector2 texcoord;
	Vector3 normal;
};

struct MaterialData {
	std::string textureFilePath;
	Vector3 diffuseColor;
	Vector3 specularColor;
	float shininess;
};

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};

class Model {
public:
	// objファイルを読み込む
	ModelData LoadObjFile(const std::string& directryPath, const std::string& filename);

	// mtlファイルを読み込む
	MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	// ゲッター
	const std::vector<VertexData>& GetVertices() { return vertices_; };

private:
	std::vector<VertexData> vertices_ = {};
};
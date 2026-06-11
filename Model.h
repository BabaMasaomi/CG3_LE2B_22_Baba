#pragma once
#include "struct.h"
#include <string>
#include <vector>

struct VertexData {
	Vector4 pos;
	Vector2 texcoord;
	Vector3 normal;
};

struct ModelData {
	std::vector<VertexData> vertices;
};

class Model {
public:
	ModelData LoadObjFile(const std::string& directryPath, const std::string& filename);

	// ゲッター
	const std::vector<VertexData>& GetVertices() { return vertices_; };

private:
	std::vector<VertexData> vertices_ = {};
};
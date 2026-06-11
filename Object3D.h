#pragma once
#include <d3d12.h>

// 前方宣言
class Model;

class Object3D {
public:
	void Initialize(Model* model);

	void Update();
	void Draw();

private:
	Model* model_;

	ID3D12Resource* vertexResource_;
	ID3D12Resource* indexResource_;
};
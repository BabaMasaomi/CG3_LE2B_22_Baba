#pragma once
#include <d3d12.h>
#include"struct.h"

class DirectXCommon;
class TrianglePSO;

class Triangle {
public:
	~Triangle();

	void Initialize(DirectXCommon* dxCommon, TrianglePSO* trianglePSO);
	// void Update();		//	今回は特に更新処理はないので、Update関数は用意しない
	void Draw();

private:
	void CreateVertexResource();
	void CreateVertexBufferView();
	void MapVertexData();

	//struct VertexData {
	//	float position[4];
	//};

	DirectXCommon* dxCommon_ = nullptr;
	TrianglePSO* trianglePSO_ = nullptr;

	ID3D12Resource* vertexResource_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	// 頂点リソースにデータを書き込む
	Vecter4* vertexData_ = nullptr;
};


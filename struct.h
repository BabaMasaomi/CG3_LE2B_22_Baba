#pragma once

// 3頂点の構造体
struct Vector3 {
	float x, y, z;
};

// 4頂点の構造体
struct Vector4 {
	float x, y, z, w;
};

// 3Dオブジェクトの変換を表す構造体
struct Transform {
    Vector3 scale;
    Vector3 rotate;
    Vector3 translate;
};

// 4x4行列の構造体
struct Matrix4x4{
    float m[4][4];
};

struct TransformationMatrix {
    Matrix4x4 WVP;
    Matrix4x4 World;
};
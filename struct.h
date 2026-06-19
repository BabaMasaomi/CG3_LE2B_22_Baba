#pragma once

// 2頂点の構造体
struct Vector2 {
	float x, y;
};

// 3頂点の構造体
struct Vector3 {
	float x, y, z;

	Vector3 operator+(const Vector3& v) const {
		return { x + v.x, y + v.y, z + v.z };
	}

	Vector3 operator-(const Vector3& v) const {
		return { x - v.x, y - v.y, z - v.z };
	}

	Vector3 operator*(float s) const {
		return { x * s, y * s, z * s };
	}

	Vector3& operator+=(const Vector3& v) {
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
}

	Vector3& operator-=(const Vector3& v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}
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

// 3x3行列の構造体
struct Matrix3x3 {
	float m[3][3];
};

// 4x4行列の構造体
struct Matrix4x4 {
	float m[4][4];
};

// 変換行列をまとめる構造体
struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};
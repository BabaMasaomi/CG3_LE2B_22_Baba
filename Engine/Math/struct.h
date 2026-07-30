#pragma once

// 2頂点の構造体
struct Vector2 {
	float x, y;
};

// 3頂点の構造体
struct Vector3 {
	float x, y, z;
	// Vector3の演算子オーバーロード
	
	/// <summary>
	/// 加算
	/// </summary>
	/// <param name="v">Vector3型変数</param>
	/// <returns>加算結果</returns>
	Vector3 operator+(const Vector3& v) const {
		return { x + v.x, y + v.y, z + v.z };
	}
	
	/// <summary>
	/// 減算
	/// </summary>
	/// <param name="v">Vector3型変数</param>
	/// <returns>減算結果</returns>
	Vector3 operator-(const Vector3& v) const {
		return { x - v.x, y - v.y, z - v.z };
	}
	
	/// <summary>
	/// スカラー倍
	/// </summary>
	/// <param name="s">スカラー値</param>
	/// <returns>スカラー倍結果</returns>
	Vector3 operator*(float s) const {
		return { x * s, y * s, z * s };
	}

	/// <summary>
	/// 加算代入演算子
	/// </summary>
	/// <param name="v">Vector3型変数</param>
	/// <returns>加算代入結果</returns>
	Vector3& operator+=(const Vector3& v) {
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	/// <summary>
	/// 減算代入演算子
	/// </summary>
	/// <param name="v">Vector3型変数</param>
	/// <returns>減算代入結果</returns>
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

// ライティングモード
enum class LightingMode {
	None,
	Lambert,
	HalfLambert,
};
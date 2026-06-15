#include "Utility.h"
#include "Logger.h"
#include <format>			// フォーマットの自動指定をしてくれる(+安全策)
#include<cmath>
#include <cassert>

/*-------------------- BufferResourceの作成 --------------------*/
Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
	HRESULT hr;

	// 頂点リソース用のヒープを生成
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;		// UploadHeapを使う
	// 頂点リソースの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	// バッファリソース。テクスチャの場合は別の設定をする
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = (sizeInBytes + 255) & ~255;;	// リソースのサイズ。引数で指定
	// バッファの場合はこれらは全て1にする
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// 返り値を用意
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	// 実際に頂点リソースを作る
	hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));

	return resource;
}

/*-------------------- DescriptorHeapの作成 --------------------*/
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {
	// ディスクリプタヒープを作成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;						// 引数で指定したタイプ
	descriptorHeapDesc.NumDescriptors = numDescriptors;		// 引数で指定した数
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	// ディスクリプタヒープが生成できなかったらエラー
	assert(SUCCEEDED(hr));
	return descriptorHeap;
}

/*-------------------- DescriptorHandleの取得 --------------------*/
// CPU
D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}
// GPU
D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

/*-------------------- std::wstringとstd::stringの変換を行う関数 --------------------*/
// string->wstring
std::wstring ConvertString(const std::string& str) {
	if (str.empty()) {
		return std::wstring();
	}

	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
	if (sizeNeeded == 0) {
		return std::wstring();
	}
	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
	return result;
}
// wstring->string
std::string ConvertString(const std::wstring& str) {
	if (str.empty()) {
		return std::string();
	}

	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
	if (sizeNeeded == 0) {
		return std::string();
	}
	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
	return result;
}

/*-------------------- 単位行列の作成関数 --------------------*/
Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 result = {};
	result.m[0][0] = 1;
	result.m[1][1] = 1;
	result.m[2][2] = 1;
	result.m[3][3] = 1;
	return result;
}

/*-------------------- アフィン変換行列を作成する関数 --------------------*/
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 result = {};
	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);
	Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);
	Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);
	Matrix4x4 rotateXYZMatrix = Multiply(rotateXMatrix, Multiply(rotateYMatrix, rotateZMatrix));
	Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);
	// 拡大縮小→回転→平行移動の順で行列を掛ける
	result = Multiply(Multiply(scaleMatrix, rotateXYZMatrix), translateMatrix);
	return result;
}

// 拡大縮小行列
Matrix4x4 MakeScaleMatrix(const Vector3& Scale) {
	Matrix4x4 result = {
		Scale.x,0.0f,0.0f,0.0f,
		0.0f,Scale.y,0.0f,0.0f,
		0.0f,0.0f,Scale.z,0.0f,
		0.0f,0.0f,0.0f,1.0f
	};
	return result;
}

// 平行移動行列
Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 result = {
		1.0f,0.0f,0.0f,0.0f,
		0.0f,1.0f,0.0f,0.0f,
		0.0f,0.0f,1.0f,0.0f,
		translate.x,translate.y,translate.z,1.0f
	};
	return result;
}

// X軸回転行列
Matrix4x4 MakeRotateXMatrix(float radian) {
	Matrix4x4 rotateXMatrix = {};
	rotateXMatrix.m[0][0] = 1.0f;
	rotateXMatrix.m[1][1] = std::cos(radian);
	rotateXMatrix.m[1][2] = std::sin(radian);
	rotateXMatrix.m[2][1] = -std::sin(radian);
	rotateXMatrix.m[2][2] = std::cos(radian);
	rotateXMatrix.m[3][3] = 1.0f;
	return rotateXMatrix;
}

// Y軸回転行列
Matrix4x4 MakeRotateYMatrix(float radian) {
	Matrix4x4 rotateYMatrix = {};
	rotateYMatrix.m[0][0] = std::cos(radian);
	rotateYMatrix.m[0][2] = -std::sin(radian);
	rotateYMatrix.m[1][1] = 1.0f;
	rotateYMatrix.m[2][0] = std::sin(radian);
	rotateYMatrix.m[2][2] = std::cos(radian);
	rotateYMatrix.m[3][3] = 1.0f;
	return rotateYMatrix;
}

// Z軸回転行列
Matrix4x4 MakeRotateZMatrix(float radian) {
	Matrix4x4 rotateZMatrix = {};
	rotateZMatrix.m[0][0] = std::cos(radian);
	rotateZMatrix.m[0][1] = std::sin(radian);
	rotateZMatrix.m[1][0] = -std::sin(radian);
	rotateZMatrix.m[1][1] = std::cos(radian);
	rotateZMatrix.m[2][2] = 1.0f;
	rotateZMatrix.m[3][3] = 1.0f;
	return rotateZMatrix;
}

/*-------------------- 逆数 --------------------*/
Matrix4x4 Inverse(const Matrix4x4& m) {
	Matrix4x4 result = {};
	float det = Determinant4x4(m);
	for (int row = 0; row < 4; row++) {
		for (int column = 0; column < 4; column++) {
			result.m[row][column] = Cofactor(m, column, row) / det;
		}
	}
	return result;
}

// 3x3の行列式の計算
float Determinant3x3(float m[3][3]) {
	return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
		- m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
		+ m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}

// 余因子の計算
float Cofactor(const Matrix4x4& m, int row, int column) {
	float subMatrix[3][3];		// 計算用の3x3の部分行列
	int subRow = 0;				// 部分行列の行インデックス
	for (int i = 0; i < 4; i++) {
		if (i == row) {
			continue;
		}
		// 部分行列の列インデックス
		int subColumn = 0;
		for (int j = 0; j < 4; j++) {
			if (j == column) {
				continue;
			}

			subMatrix[subRow][subColumn] = m.m[i][j];
			subColumn++;
		}
		subRow++;	// 部分行列の行インデックスを進める
	}
	float result = Determinant3x3(subMatrix);
	if ((row + column) % 2 == 1) {
		result *= -1.0f;
	}
	return result;
}

// 4x4の行列式の計算
float Determinant4x4(const Matrix4x4& m) {
	float det = 0.0f;
	for (int column = 0; column < 4; column++) {
		det += m.m[0][column] * Cofactor(m, 0, column);
	}
	return det;
}

/*-------------------- 透視投影行列 --------------------*/
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	Matrix4x4 result = {};
	float cot = 1.0f / tanf(fovY / 2.0f);
	result.m[0][0] = cot / aspectRatio;
	result.m[1][1] = cot;
	result.m[2][2] = farClip / (farClip - nearClip);
	result.m[2][3] = 1.0f;
	result.m[3][2] = -nearClip * farClip / (farClip - nearClip);
	return result;
}

/*-------------------- 平行投影行列 --------------------*/
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
	Matrix4x4 result = {};
	result.m[0][0] = 2.0f / (right - left);
	result.m[1][1] = 2.0f / (top - bottom);
	result.m[2][2] = 1.0f / (farClip - nearClip);
	result.m[3][0] = (left + right) / (left - right);
	result.m[3][1] = (top + bottom) / (bottom - top);
	result.m[3][2] = nearClip / (nearClip - farClip);
	result.m[3][3] = 1.0f;
	return result;
}

/*-------------------- 乗算 --------------------*/
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result = {};
	for (int row = 0; row < 4; row++) {
		for (int column = 0; column < 4; column++) {
			result.m[row][column] =
				m1.m[row][0] * m2.m[0][column] +
				m1.m[row][1] * m2.m[1][column] +
				m1.m[row][2] * m2.m[2][column] +
				m1.m[row][3] * m2.m[3][column];
		}
	}
	return result;
}

/*-------------------- ビューポート変換行列 --------------------*/
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 result = {};
	result.m[0][0] = width / 2.0f;
	result.m[1][1] = -height / 2.0f;
	result.m[2][2] = (maxDepth - minDepth);
	result.m[3][0] = left + width / 2.0f;
	result.m[3][1] = top + height / 2.0f;
	result.m[3][2] = minDepth;
	result.m[3][3] = 1.0f;
	return result;
}

/*-------------------- 転置行列 --------------------*/
Matrix4x4 Transpose(const Matrix4x4& m) {
	Matrix4x4 result{};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.m[i][j] = m.m[j][i];
		}
	}
	return result;
}

/*-------------------- 座標変換 --------------------*/
Vector3 CoordinateTransform(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result;
	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + matrix.m[3][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + matrix.m[3][1];
	result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + matrix.m[3][2];
	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + matrix.m[3][3];
	// 0除算の防止
	assert(w != 0.0f);
	result.x /= w;
	result.y /= w;
	result.z /= w;
	return result;
}
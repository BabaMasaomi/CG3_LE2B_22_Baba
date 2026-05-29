#pragma once
#include <d3d12.h>
#include <string>
#include"struct.h"

// BufferResourceの作成関数
ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);

// DescriptorHeapの作成関数
ID3D12DescriptorHeap* CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

// DescriptorHandleの取得
// CPU
D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);
// GPU
D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);

// std::wstringとstd::stringの変換を行う関数
// string->wstring
std::wstring ConvertString(const std::string& str);
// wstring->string
std::string ConvertString(const std::wstring& str);

// 単位行列の作成関数
Matrix4x4 MakeIdentity4x4();

// アフィン変換行列を作成する関数
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);
Matrix4x4 MakeScaleMatrix(const Vector3& Scale);			// 拡大縮小行列
Matrix4x4 MakeTranslateMatrix(const Vector3& translate);	// 平行移動行列
Matrix4x4 MakeRotateXMatrix(float radian);					// X軸回転行列
Matrix4x4 MakeRotateYMatrix(float radian);					// Y軸回転行列
Matrix4x4 MakeRotateZMatrix(float radian);					// Z軸回転行列

// 逆数
Matrix4x4 Inverse(const Matrix4x4& m);
float Determinant3x3(float m[3][3]);						// 3x3の行列式の計算
float Cofactor(const Matrix4x4& m, int row, int column);	// 余因子の計算
float Determinant4x4(const Matrix4x4& m);					// 4x4の行列式の計算

// 透視投影行列
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

// 平行投影行列
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

// 行列の積
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);

// ビューポート変換行列
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);

// 転置行列
Matrix4x4 Transpose(const Matrix4x4& m);

// 座標変換行列
Vector3 CoordinateTransform(const Vector3& vector, const Matrix4x4& matrix);
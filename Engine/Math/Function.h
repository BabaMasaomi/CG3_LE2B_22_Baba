#pragma once
#include <d3d12.h>
#include <string>
#include <wrl.h>
#include"struct.h"

namespace Engine {

	/*==================== メンバ関数の宣言 ====================*/
	/// <summary>
	/// BufferResourceの作成関数
	/// </summary>
	/// <param name="device">D3D12デバイス</param>
	/// <param name="sizeInBytes">バッファのサイズ（バイト）</param>
	/// <returns>作成されたBufferResource</returns>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);

	/// <summary>
	/// DescriptorHeapの作成関数
	/// </summary>
	/// <param name="device">D3D12デバイス</param>
	/// <param name="heapType">デスクリプターヒープのタイプ</param>
	/// <param name="numDescriptors">デスクリプタの数</param>
	/// <param name="shaderVisible">シェーダーからアクセス可能か</param>
	/// <returns>作成されたDescriptorHeap</returns>
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	/// <summary>
	/// CPUのDescriptorHandleの取得関数
	/// </summary>
	/// <param name="descriptorHeap">デスクリプターヒープ</param>
	/// <param name="descriptorSize">デスクリプタのサイズ</param>
	/// <param name="index">インデックス</param>
	/// <returns>取得したDescriptorHandle</returns>
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);
	/// <summary>
	/// GPUのDescriptorHandleの取得関数
	/// </summary>
	/// <param name="descriptorHeap">デスクリプターヒープ</param>
	/// <param name="descriptorSize">デスクリプタのサイズ</param>
	/// <param name="index">インデックス</param>
	/// <returns>取得したDescriptorHandle</returns>
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);

	// std::wstringとstd::stringの変換を行う関数

	/// <summary>
	/// std::stringをstd::wstringに変換する関数
	/// </summary>
	/// <param name="str">変換する文字列</param>
	/// <returns>変換された文字列</returns>
	std::wstring ConvertString(const std::string& str);
	/// <summary>
	/// std::wstringをstd::stringに変換する関数
	/// </summary>
	/// <param name="str">変換する文字列</param>
	/// <returns>変換された文字列</returns>
	std::string ConvertString(const std::wstring& str);

	/// <summary>
	/// 単位行列の作成関数
	/// </summary>
	/// <returns>単位行列</returns>
	Matrix4x4 MakeIdentity4x4();

	/// <summary>
	/// アフィン変換行列を作成する関数
	/// </summary>
	/// <param name="scale">拡大縮小</param>
	/// <param name="rotate">回転</param>
	/// <param name="translate">平行移動</param>
	/// <returns>作成されたアフィン変換行列</returns>
	Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);
	/// <summary>
	/// 拡大縮小行列を作成する関数
	/// </summary>
	/// <param name="Scale">拡大縮小</param>
	/// <returns>作成された拡大縮小行列</returns>
	Matrix4x4 MakeScaleMatrix(const Vector3& Scale);
	/// <summary>
	/// 平行移動行列を作成する関数
	/// </summary>
	/// <param name="translate">平行移動</param>
	/// <returns>作成された平行移動行列</returns>
	Matrix4x4 MakeTranslateMatrix(const Vector3& translate);
	/// <summary>
	/// X軸回転行列を作成する関数
	/// </summary>
	/// <param name="radian">回転角度（ラジアン）</param>
	/// <returns>作成されたX軸回転行列</returns>
	Matrix4x4 MakeRotateXMatrix(float radian);
	/// <summary>
	/// Y軸回転行列を作成する関数
	/// </summary>
	/// <param name="radian">回転角度（ラジアン）</param>
	/// <returns>作成されたY軸回転行列</returns>
	Matrix4x4 MakeRotateYMatrix(float radian);
	/// <summary>
	/// Z軸回転行列を作成する関数
	/// </summary>
	/// <param name="radian">回転角度（ラジアン）</param>
	/// <returns>作成されたZ軸回転行列</returns>
	Matrix4x4 MakeRotateZMatrix(float radian);

	/// <summary>
	/// 逆行列を作成する関数
	/// </summary>
	/// <param name="m">入力行列</param>
	/// <returns>逆行列</returns>
	Matrix4x4 Inverse(const Matrix4x4& m);
	/// <summary>
	/// 3x3の行列式を計算する関数
	/// </summary>
	/// <param name="m">入力行列</param>
	/// <returns>行列式</returns>
	float Determinant3x3(float m[3][3]);
	/// <summary>
	/// 余因子を計算する関数
	/// </summary>
	/// <param name="m">入力行列</param>
	/// <param name="row">行</param>
	/// <param name="column">列</param>
	/// <returns>余因子</returns>
	float Cofactor(const Matrix4x4& m, int row, int column);
	/// <summary>
	/// 4x4の行列式を計算する関数
	/// </summary>
	/// <param name="m">入力行列</param>
	/// <returns>行列式</returns>
	float Determinant4x4(const Matrix4x4& m);

	/// <summary>
	/// 透視投影行列を作成する関数
	/// </summary>
	/// <param name="fovY">垂直視野角（ラジアン）</param>
	/// <param name="aspectRatio">アスペクト比</param>
	/// <param name="nearClip">近平面</param>
	/// <param name="farClip">遠平面</param>
	/// <returns>作成された透視投影行列</returns>
	Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

	/// <summary>
	/// 平行投影行列を作成する関数
	/// </summary>
	/// <param name="left">左端</param>
	/// <param name="top">上端</param>
	/// <param name="right">右端</param>
	/// <param name="bottom">下端</param>
	/// <param name="nearClip">近平面</param>
	/// <param name="farClip">遠平面</param>
	/// <returns>作成された平行投影行列</returns>
	Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

	/// <summary>
	/// 2つの行列の積を計算する関数
	/// </summary>
	/// <param name="m1">最初の行列</param>
	/// <param name="m2">2番目の行列</param>
	/// <returns>積</returns>
	Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);

	/// <summary>
	/// ビューポート変換行列を作成する関数
	/// </summary>
	/// <param name="left">左端</param>
	/// <param name="top">上端</param>
	/// <param name="width">幅</param>
	/// <param name="height">高さ</param>
	/// <param name="minDepth">最小深度</param>
	/// <param name="maxDepth">最大深度</param>
	/// <returns>作成されたビューポート変換行列</returns>
	Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);

	/// <summary>
	/// 転置行列を作成する関数
	/// </summary>
	/// <param name="m">入力行列</param>
	/// <returns>転置行列</returns>
	Matrix4x4 Transpose(const Matrix4x4& m);

	/// <summary>
	/// 座標を変換する関数
	/// </summary>
	/// <param name="vector">入力ベクトル</param>
	/// <param name="matrix">変換行列</param>
	/// <returns>変換されたベクトル</returns>
	Vector3 CoordinateTransform(const Vector3& vector, const Matrix4x4& matrix);

	/// <summary>
	/// 法線を変換する関数
	/// </summary>
	/// <param name="v">入力法線ベクトル</param>
	/// <param name="m">変換行列</param>
	/// <returns>変換された法線ベクトル</returns>
	Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m);
}
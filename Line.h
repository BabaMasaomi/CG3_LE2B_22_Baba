#pragma once

#include "struct.h"

#include <d3d12.h>
#include <wrl.h>

namespace Engine {

	// 前方宣言
	class Camera;
	class DirectXCommon;
	class LinePSO;

	class Line {
	public:
		/// <summary>
		/// 線分の初期化
		/// </summary>
		/// <param name="dxCommon">DirectXCommon</param>
		/// <param name="linePSO">線分用PSO</param>
		/// <param name="start">始点</param>
		/// <param name="end">終点</param>
		void Initialize(
			DirectXCommon* dxCommon,
			LinePSO* linePSO,
			const Vector3& start,
			const Vector3& end
		);

		/// <summary>
		/// 線分の更新
		/// </summary>
		void Update(const Camera& camera);

		/// <summary>
		/// 線分の描画
		/// </summary>
		void Draw();

		/*==================== アクセッサ ====================*/

		const Vector3& GetStart() const {
			return start_;
		}

		const Vector3& GetEnd() const {
			return end_;
		}

		const Vector4& GetColor() const {
			return color_;
		}

		void SetStart(const Vector3& start) {
			start_ = start;
		}

		void SetEnd(const Vector3& end) {
			end_ = end;
		}

		void SetColor(const Vector4& color) {
			color_ = color;
		}

	private:
		/// <summary>
		/// 頂点リソースを作成
		/// </summary>
		void CreateVertexResource();

		/// <summary>
		/// 頂点データを更新
		/// </summary>
		void UpdateVertexData();

		/// <summary>
		/// 変換行列用リソースを作成
		/// </summary>
		void CreateTransformationResource();

		/// <summary>
		/// 色用リソースを作成
		/// </summary>
		void CreateMaterialResource();

	private:
		// 線分用頂点
		struct VertexData {
			Vector4 position;
		};

		// 線分用マテリアル
		struct LineMaterial {
			Vector4 color;
		};

		// DirectXCommon
		DirectXCommon* dxCommon_ = nullptr;

		// 線分用PSO
		LinePSO* linePSO_ = nullptr;

		// 始点
		Vector3 start_ = {
			0.0f,
			0.0f,
			0.0f
		};

		// 終点
		Vector3 end_ = {
			1.0f,
			0.0f,
			0.0f
		};

		// 線分の色
		Vector4 color_ = {
			1.0f,
			1.0f,
			1.0f,
			1.0f
		};

		// 頂点リソース
		Microsoft::WRL::ComPtr<ID3D12Resource>
			vertexResource_ = nullptr;

		// 頂点バッファビュー
		D3D12_VERTEX_BUFFER_VIEW
			vertexBufferView_{};

		// 頂点データ
		VertexData* vertexData_ = nullptr;

		// 変換行列リソース
		Microsoft::WRL::ComPtr<ID3D12Resource>
			transformationResource_ = nullptr;

		// 変換行列データ
		TransformationMatrix*
			transformationData_ = nullptr;

		// 色用リソース
		Microsoft::WRL::ComPtr<ID3D12Resource>
			materialResource_ = nullptr;

		// 色データ
		LineMaterial* materialData_ = nullptr;
	};

}
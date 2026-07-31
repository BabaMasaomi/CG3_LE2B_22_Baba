#include "Model.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "Utility.h"
#include <fstream>
#include <sstream>
#include <cassert>

namespace Engine {

	/*-------------------- OBJファイルから初期化 --------------------*/
	void Model::Initialize(DirectXCommon* dxCommon, TextureManager* textureManager, const std::string& filename) {
		// ポインタを保存しておく
		dxCommon_ = dxCommon;
		textureManager_ = textureManager;

		// モデル読み込み
		LoadObjFile(filename);

		// テクスチャの読み込み
		textureIndex_ = textureManager_->LoadTexture(modelData_.material.textureFilePath);

		// 頂点、インデックス、マテリアル等を生成
		CreateResources();
	}

	/*-------------------- プログラムで生成したModelDataから初期化 --------------------*/
	void Model::Initialize(DirectXCommon* dxCommon, TextureManager* textureManager, ModelData modelData) {
		// ポインタを保存しておく
		dxCommon_ = dxCommon;
		textureManager_ = textureManager;

		// 生成済みのモデルデータを受け取る
		modelData_ = std::move(modelData);

		// テクスチャ読み込み
		textureIndex_ = textureManager_->LoadTexture(modelData_.material.textureFilePath);

		// 頂点、インデックス、マテリアル等を生成
		CreateResources();
	}

	/*-------------------- objファイルを読み込む --------------------*/
	void Model::LoadObjFile(const std::string& filename) {
		// 1.中で必要となる変数の宣言
		std::vector<Vector4> positions;	// 位置
		std::vector<Vector3> normals;	// 法線
		std::vector<Vector2> texcoords;	// テクスチャ座標
		std::string line;				// ファイルから読んだ1行を格納するもの

		// 2.ファイルを開く
		std::ifstream file("resources/Textures/" + filename);	// ファイルを開く
		assert(file.is_open());	// 開けなかったら止める

		// 3.実際にファイルを読み、ModelDataを構築していく
		while (std::getline(file, line)) {
			std::string identifier;
			std::istringstream s(line);
			s >> identifier;	// 先頭の識別子を読む

			// identifierに応じた処理
			// 頂点情報を読む
			if (identifier == "v") {
				Vector4 position;
				s >> position.x >> position.y >> position.z;
				position.w = 1.0f;
				positions.push_back(position);

			} else if (identifier == "vt") {
				Vector2 texcoord;
				s >> texcoord.x >> texcoord.y;
				texcoords.push_back(texcoord);

			} else if (identifier == "vn") {
				Vector3 normal;
				s >> normal.x >> normal.y >> normal.z;
				normals.push_back(normal);

			} else if (identifier == "f") {
				// 三角形を作る
				VertexData triangle[3];
				// 面は三角形限定、その他は未対応
				for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
					std::string vertexDefinition;
					s >> vertexDefinition;
					// 頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
					std::istringstream v(vertexDefinition);
					uint32_t elementIndices[3];
					for (int32_t element = 0; element < 3; ++element) {
						std::string index;
						std::getline(v, index, '/');	// 区切りでインデックスを読んでいく
						elementIndices[element] = std::stoi(index);
					}
					// 各要素のIndexから、実際の要素の値を取得して、頂点を構築する
					Vector4 position = positions[elementIndices[0] - 1];
					Vector2 texcoord = texcoords[elementIndices[1] - 1];
					Vector3 normal = normals[elementIndices[2] - 1];
					VertexData vertex = { position,texcoord,normal };
					position.x *= -1.0f;
					texcoord.y = 1.0f - texcoord.y;
					normal.x *= -1.0f;
					triangle[faceVertex] = { position,texcoord,normal };
				}
				modelData_.vertices.push_back(triangle[2]);
				modelData_.vertices.push_back(triangle[1]);
				modelData_.vertices.push_back(triangle[0]);

			} else if (identifier == "mtllib") {
				// materialTemplateLibraryファイルの名前を取得する
				std::string materialFilename;
				s >> materialFilename;
				// 基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
				modelData_.material = LoadMaterialTemplateFile(materialFilename);
			}
		}
	}

	/*-------------------- mtlファイルを読み込む --------------------*/
	MaterialData Model::LoadMaterialTemplateFile(const std::string& filename) {
		// 1.中で必要となる変数の宣言
		MaterialData materialData;	// 構築するMaterialData
		std::string line;			// ファイルから読んだ1行を格納するもの

		// 2.ファイルを開く
		std::ifstream file("resources/Textures/" + filename);	// ファイルを開く
		assert(file.is_open());								// 開けなかったら止める

		// 3.実際にファイルを読み、MaterialDataを構築していく
		while (std::getline(file, line)) {
			std::string identifier;
			std::istringstream s(line);
			s >> identifier;

			// identifierに応じた処理
			if (identifier == "map_Kd") {
				std::string textureFilename;
				s >> textureFilename;
				// 連結してファイルパスにする
				materialData.textureFilePath = "resources/Textures/" + textureFilename;

			} else if (identifier == "Kd") {
				s >> materialData.diffuseColor.x >> materialData.diffuseColor.y >> materialData.diffuseColor.z;

			} else if (identifier == "Ks") {
				s >> materialData.specularColor.x >> materialData.specularColor.y >> materialData.specularColor.z;

			} else if (identifier == "Ns") {
				s >> materialData.shininess;

			}
		}

		// 4.MaterialDataを返す
		return materialData;
	}

	/*-------------------- UV変換行列の更新 --------------------*/
	void Model::UpdateUVTransform(const Matrix4x4& uvTransform) {
		materialData_->uvTransform = uvTransform;
	}

	/*-------------------- リソース作成をまとめる --------------------*/
	void Model::CreateResources() {
		// VertexResourceとVertexBufferViewを用意
		CreateVertexResource();
		CreateVertexBufferView();
		MapVertexData();

		// Index生成がないのでコメントアウト
		//CreateIndexResource();
		//CreateIndexBufferView();
		//MapIndexData();

		// 色付けのためのMaterialResourceも作る
		CreateMaterialResource();
		MapMaterial();
	}

	/*-------------------- VertexResourceの生成 --------------------*/
	void Model::CreateVertexResource() {
		vertexResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * modelData_.vertices.size());
	}

	/*-------------------- VertexBufferViewの作成 --------------------*/
	void Model::CreateVertexBufferView() {
		// リソースの先頭のアドレスから使う
		vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
		// 使用するリソースのサイズはモデルデータに依存
		vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
		// 1頂点あたりのサイズ
		vertexBufferView_.StrideInBytes = sizeof(VertexData);
	}

	/*-------------------- VertexResourceにデータを書き込む --------------------*/
	void Model::MapVertexData() {
		// 頂点リソースにデータを書きこむ
		VertexData* vertexData = nullptr;
		// 書き込むためのアドレスを取得
		vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
		// 頂点データをリソースにコピー
		std::memcpy(vertexData, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
	}

	/*-------------------- MaterialResourceの生成 --------------------*/
	void Model::CreateMaterialResource() {
		materialResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(Material));
	}

	/*-------------------- MaterialResourceにデータを書き込む --------------------*/
	void Model::MapMaterial() {
		// Mapする
		materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
		std::memset(materialData_, 0, sizeof(Material));
		// ここで色を弄れる
		materialData_->color = { modelData_.material.diffuseColor.x, modelData_.material.diffuseColor.y, modelData_.material.diffuseColor.z, 1.0f };
		materialData_->enableLighting = true;
		materialData_->uvTransform = MakeIdentity4x4();
	}

	// ゲッターを定義(ヘッダー側だとTextureManagerを含む定義ができない)
	D3D12_GPU_DESCRIPTOR_HANDLE Model::GetTextureHandle() const {
		return textureManager_->GetTextureSrvHandleGPU(textureIndex_ - 1);
	}
}
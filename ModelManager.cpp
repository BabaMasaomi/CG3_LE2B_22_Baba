#include "ModelManager.h"
#include "DirectXCommon.h"
#include "TextureManager.h"

/*-------------------- 初期化 --------------------*/
void ModelManager::Initialize(DirectXCommon* dxCommon, TextureManager* textureManager) {
	dxCommon_ = dxCommon;
	textureManager_ = textureManager;
}

///*-------------------- モデル読み込み --------------------*/
//void ModelManager::LoadModel() {
//
//}
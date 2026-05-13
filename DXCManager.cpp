#include <Windows.h>

#include "DXCManager.h"
#include "Logger.h"
#include <format>			// フォーマットの自動指定をしてくれる(+安全策)
#include <cassert>

// デストラクタ
DXCManager::~DXCManager() {
	if (includeHandler_) includeHandler_->Release();
	if (dxcCompiler_) dxcCompiler_->Release();
	if (dxcUtils_) dxcUtils_->Release();
}

/*-------------------- DXCの初期化 --------------------*/
void DXCManager::Initialize() {
	HRESULT hr;

	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
	assert(SUCCEEDED(hr));

	// includeの為に設定
	hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	assert(SUCCEEDED(hr));
}

/*-------------------- シェーダーのコンパイル --------------------*/
IDxcBlob* DXCManager::CompileShader(
	// CompilerするShaderファイルへのパス
	const std::wstring& filePath,
	// CompilerするShaderのprofile
	const wchar_t* profile,
	// 初期化で生成したもの3つ
	IDxcUtils* dxcUtils,
	IDxcCompiler3* dxcCompiler,
	IDxcIncludeHandler* includeHandler)
{
	/// 1.hlslファイルを読む
	// これからシェーダーをコンパイルすることをログに出す
	Log(ConvertString(std::format(L"Begin Compiling, path:{}, profile:{}", filePath, profile)));
	// hlslファイルを読む
	IDxcBlobEncoding* shaderSource_ = nullptr;
	HRESULT hr = dxcUtils_->LoadFile(filePath.c_str(), nullptr, &shaderSource_);
	//読めなかったら止める
	assert(SUCCEEDED(hr));
	// 読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer{};
	shaderSourceBuffer.Ptr = shaderSource_->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource_->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_ACP;

	/// 2.Compileする
	LPCWSTR arguments[] = {
		filePath.c_str(),			// コンパイルするhlslファイル名
		L"-E", L"main",				// エントリーポイントはmain関数
		L"-T", profile,				// shaderProfileの設定
		L"-Zi",L"-Qembed_debug",	// デバッグ用の情報を埋め込む
		L"-Od",						// 最適化なし
		L"-Zpr",					// メモリレイアウトは行優先
	};

	// 実際にShaderをコンパイルする
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer,			// 読み込んだファイル
		arguments,						// コンパイルオプション
		_countof(arguments),			// コンパイルオプションの数
		includeHandler,					// includeが含まれた諸々
		IID_PPV_ARGS(&shaderResult)		// コンパイル結果
	);
	// dxcが起動できないのは致命的
	assert(SUCCEEDED(hr));

	/// 3.警告・エラーが出ていないか確認する
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(shaderError->GetStringPointer());
		// 警告・エラーダメゼッタイ
		assert(false);
	}

	/// 4.Compile結果を受け取って返す
	// コンパイル結果から実用用のバイナリ部分を取得
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	// 成功したログを出す
	Log(ConvertString(std::format(L"Compile Succeded, path:{}, profile:{}\n", filePath, profile)));
	// もう使わないリソースを解放
	if (shaderSource_) shaderSource_->Release();
	if (shaderResult) shaderResult->Release();
	// 実行用バイナリを返却
	return shaderBlob;
}
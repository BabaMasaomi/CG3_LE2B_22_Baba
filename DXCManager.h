#pragma once
#include<dxcapi.h>
#include <string>

#pragma comment(lib,"dxcompiler.lib")

class DXCManager {
public:
	~DXCManager();

	void Initialize();

	IDxcBlob* CompileShader(
		const std::wstring& filePath,
		const wchar_t* profile,
		IDxcUtils* dxcUtils,
		IDxcCompiler3* dxcCompiler,
		IDxcIncludeHandler* includeHandler
	);

	// ゲッター
	IDxcUtils* GetDxcUtils() const { return dxcUtils_; }
	IDxcCompiler3* GetDxcCompiler() const { return dxcCompiler_; }
	IDxcIncludeHandler* GetIncludeHandler() const { return includeHandler_; }

private:
	// dxCompilerの初期化
	IDxcUtils* dxcUtils_ = nullptr;
	IDxcCompiler3* dxcCompiler_ = nullptr;
	IDxcIncludeHandler* includeHandler_ = nullptr;
};
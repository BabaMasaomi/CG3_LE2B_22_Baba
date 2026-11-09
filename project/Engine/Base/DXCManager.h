#pragma once
#include <dxcapi.h>
#include <string>
#include <wrl.h>

// libのリンク
#pragma comment(lib,"dxcompiler.lib")

namespace Engine {

	class DXCManager {
	public:
		/*==================== メンバ関数とデストラクタの宣言 ====================*/
		// デストラクタ
		~DXCManager();

		/// <summary>
		/// DXCManagerの初期化
		/// </summary>
		void Initialize();

		/// <summary>
		/// シェーダーのコンパイル
		/// </summary>
		/// <param name="filePath">ファイルパス</param>
		/// <param name="profile">プロファイル</param>
		/// <param name="dxcUtils">DXCユーティリティ</param>
		/// <param name="dxcCompiler">DXCコンパイラ</param>
		/// <param name="includeHandler">インクルードハンドラ</param>
		/// <returns>コンパイルされたシェーダーのblob</returns>
		Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
			const std::wstring& filePath,
			const wchar_t* profile,
			IDxcUtils* dxcUtils,
			IDxcCompiler3* dxcCompiler,
			IDxcIncludeHandler* includeHandler
		);

		/*==================== アクセッサ ====================*/
		// ゲッター
		IDxcUtils* GetDxcUtils() const { return dxcUtils_.Get(); }
		IDxcCompiler3* GetDxcCompiler() const { return dxcCompiler_.Get(); }
		IDxcIncludeHandler* GetIncludeHandler() const { return includeHandler_.Get(); }

	private:
		/*==================== メンバ変数 ====================*/
		// dxCompilerの初期化
		Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_ = nullptr;
		Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_ = nullptr;
		Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_ = nullptr;
	};
}
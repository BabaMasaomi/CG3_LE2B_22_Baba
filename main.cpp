#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>	// フォーマットの自動指定をしてくれる(+安全策)
#include <filesystem>
// ファイルを読んだり書いたりするためのライブラリ
#include <fstream>
// 時間を扱うためのライブラリ
#include <chrono>
// DirectXを使うためのライブラリ
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
// Debug用のあれこれを使える様にする
#include <dbghelp.h>
#include <strsafe.h>

// libのリンク
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "Dbghelp.lib")

/*-------------------- 関数 --------------------*/
#pragma region
static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exceptionPointers) {
	// 時刻を取得して、時刻を名前に入れたファイルを作成。Dumpsディレクトリ以下に保存する
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = { 0 };
	CreateDirectory(L"./Dumps", nullptr);
	StringCchPrintfW(filePath, MAX_PATH, L"./Dumps/%04d-%02d%02d-%02d%02d.dmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);
	HANDLE dumpFileHandle = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
	// processID(このexeのID)とクラッシュ(例外)の発生したthreadIDを取得
	DWORD processID = GetCurrentProcessId();
	DWORD threadID = GetCurrentThreadId();
	// 設定情報を入力
	MINIDUMP_EXCEPTION_INFORMATION minidumpinfomation{ 0 };
	minidumpinfomation.ThreadId = threadID;
	minidumpinfomation.ExceptionPointers = exceptionPointers;
	minidumpinfomation.ClientPointers = TRUE;
	// Dumpを出力。MiniDumpNormalは最低限の情報を出力するフラグ
	MiniDumpWriteDump(GetCurrentProcess(), processID, dumpFileHandle, MiniDumpNormal, &minidumpinfomation, nullptr, nullptr);
	// 他に関連づけられているSEH例外ハンドラがあれば実行。通常プロセスは終了する
	return EXCEPTION_EXECUTE_HANDLER;
}

// wstring->stringの変換関数
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

// 出力ウィンドウに文字列を表示できる様にする関数
void Log(const std::string& message) {
	OutputDebugStringA(message.c_str());
}

void Log(const std::wstring& message) {
	Log(ConvertString(message));
}

// Logファイルを出力できる様にする関数
void Log(std::ofstream& os, const std::string& message) {
	os << message << std::endl;
	OutputDebugStringA(message.c_str());
}
#pragma endregion

/*-------------------- ウィンドウプロシージャ --------------------*/
#pragma region
// ウィンドウプロシージャ(ゲームウィンドウの表示に使用)
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	switch (msg) {
		// ウィンドウが破棄されたときの処理
	case WM_DESTROY:
		// OSに対して、アプリケーションの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	// 標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	// 誰も捕捉しなかった場合に、捕捉する関数を登録
	SetUnhandledExceptionFilter(ExportDump);
#pragma endregion

	/*-------------------- ゲームウィンドウの表示処理 --------------------*/
#pragma region
	WNDCLASS wc{};
	// ウィンドウプロシージャ
	wc.lpfnWndProc = WindowProc;
	// ウィンドウクラス名
	wc.lpszClassName = L"CG2WindowClass";
	// インスタンスハンドル
	wc.hInstance = GetModuleHandle(nullptr);
	// カーソル
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	// ウィンドウクラスの登録
	RegisterClass(&wc);

	// クライアント領域サイズ
	const int32_t kClientWidth = 1280;
	const int32_t kClientHeight = 720;

	// ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0,0,kClientWidth,kClientHeight };

	// クライアント領域をもとに実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// ウィンドウの生成
	HWND hwnd = CreateWindow(
		wc.lpszClassName,		// 利用するクラス名
		L"CG2",					// タイトルバーの文字
		WS_OVERLAPPEDWINDOW,	// よく見るウィンドウスタイル
		CW_USEDEFAULT,			// 表示X座標(Windowsに任せる)
		CW_USEDEFAULT,			// 表示Y座標(Windowsに任せる)
		wrc.right - wrc.left,	// ウィンドウ横幅
		wrc.bottom - wrc.top,	// ウィンドウ縦幅
		nullptr,				// 親ウィンドウハンドル
		nullptr,				// メニューハンドル
		wc.hInstance,			// インスタンスハンドル
		nullptr);				// オプション

	// ウィンドウを表示する
	ShowWindow(hwnd, SW_SHOW);
#pragma endregion

	/*-------------------- DirectX初期化 --------------------*/
#pragma region
	// DXGIファクトリーの生成
	IDXGIFactory7* dxgiFactory = nullptr;
	// HRESULT型はWindows系のエラーコードで、成功したか失敗したかを表す。SUCCEEDEDマクロで成功したかどうかを判定できる。
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	// 初期化の根本部分でエラーが発生したら、プログラムが間違ってるかどうにもできない場合があるのでassertにしておく
	assert(SUCCEEDED(hr));

	// 使用するアダプタ用の変数(最初はnullptrで初期化)
	IDXGIAdapter4* useAdapter = nullptr;
	// 良い順にアダプタを頼む
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) !=
		DXGI_ERROR_NOT_FOUND; ++i) {
		// アダプタの情報を取得
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));	// 取得できないのは一大事
		// ソフトウェアアダプタでなければ採用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			// 採用したアダプタの情報をログに出す(wstringの方)
			Log(std::format(L"Use Adapter: {}\n", adapterDesc.Description));
			break;
		}
		useAdapter = nullptr;		// ソフトウェアアダプタだったので、使わない
	}
	// 良いアダプタが見つからなかったら、エラー
	assert(useAdapter != nullptr);

	// D3D12デバイスの生成
	ID3D12Device* device = nullptr;
	// 機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelString[] = { "12.2","12.1","12.0" };
	// 高い順に生成できるか試す
	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		// 採用したアダプターデバイスを生成
		hr = D3D12CreateDevice(useAdapter, featureLevels[i], IID_PPV_ARGS(&device));
		// 指定した機能レベルで生成できたか確認	
		if (SUCCEEDED(hr)) {
			// 生成できたのでログ出力を行ってループを抜ける
			Log(std::format("Feature Level {}\n", featureLevelString[i]));
			break;
		}
	}

	// デバイスが生成できなかったらエラー
	assert(device != nullptr);
	Log("Complete create D3D12 device!\n");	// 初期化完了のログを出す
#pragma endregion

	/*-------------------- ログファイルの準備 --------------------*/
#pragma region
	// Logのディレクトリを用意
	std::filesystem::create_directories("logs");

	// 現在時刻を取得
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

	// コンマ何秒かはいらないので、秒単位に変換
	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);

	// 日本時刻に変換
	std::chrono::zoned_time localTime{ std::chrono::current_zone(), nowSeconds };

	// formatを使って年月日_時分秒の文字列を作る
	std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);

	// 時刻を使ってログファイルの名前を作る
	std::string dateFilePath = std::string("logs/") + dateString + ".log";

	// ファイルを作って書き込み準備
	std::ofstream logStream(dateFilePath);
#pragma endregion

	/*-------------------- ゲームループの処理 --------------------*/

	MSG msg{};
	// ウィンドウのxボタンが押されるまでループ
	while (msg.message != WM_QUIT) {
		// Windowsにメッセージが来ていたら最優先で処理する
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			uint32_t* p = nullptr;
			*p = 100;	// 故意にクラッシュさせるコード(動作確認用)

		} else {
			// ゲームの処理

		}
	}

	return 0;
}
#include <Windows.h>
#include "WinApp.h"
// ImGuiの表示に必要
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_win32.h"

// ImGuiのウィンドウプロシージャ
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

// デストラクタ 
WinApp::~WinApp() {
    CloseWindow(hwnd_);
}

// ウィンドウプロシージャ(ゲームウィンドウの表示に使用)
LRESULT CALLBACK WindowProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam) {

	// ImGuiのウィンドウプロシージャを呼び出す
#ifdef USE_IMGUI
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
        return true;
    }
#endif

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

/*-------------------- 初期化 --------------------*/
void WinApp::Initialize() {
	// ImGuiのバージョンチェック
#ifdef USE_IMGUI
    IMGUI_CHECKVERSION();
#endif

    WNDCLASS wc{};
    // ウィンドウプロシージャ
    wc.lpfnWndProc = WindowProc;
    // ウィンドウクラス名
    wc.lpszClassName = L"CG2WindowClass";
    // インスタンスハンドル
    wc.hInstance = GetModuleHandle(nullptr);
    // カーソル
    RegisterClass(&wc);

    // ウィンドウサイズを表す構造体にクライアント領域を入れる
    RECT wrc = { 0, 0, kClientWidth_, kClientHeight_ };

    // クライアント領域をもとに実際のサイズにwrcを変更してもらう
    AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

    // ウィンドウの生成
    hwnd_ = CreateWindow( 
        wc.lpszClassName,       // 利用するクラス名
        L"CG2",                 // タイトルバーの文字
        WS_OVERLAPPEDWINDOW,    // よく見るウィンドウスタイル
        CW_USEDEFAULT,          // 表示X座標(Windowsに任せる)
        CW_USEDEFAULT,          // 表示Y座標(Windowsに任せる)
        wrc.right - wrc.left,   // ウィンドウ横幅
        wrc.bottom - wrc.top,   // ウィンドウ縦幅
        nullptr,                // 親ウィンドウハンドル
        nullptr,                // メニューハンドル
        wc.hInstance,           // インスタンスハンドル
        nullptr                 // オプション
    );

    // ウィンドウを表示する
    ShowWindow(hwnd_, SW_SHOW);
}

/*-------------------- メッセージ処理 --------------------*/
void WinApp::ProcessMessage() {
    // Windowsにメッセージが来ていたら最優先で処理する
    if (PeekMessage(&msg_, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg_);
        DispatchMessage(&msg_);
    }
}

/*-------------------- ゲーム終了判定 --------------------*/
bool WinApp::IsEndGame() {
    return msg_.message == WM_QUIT;
}
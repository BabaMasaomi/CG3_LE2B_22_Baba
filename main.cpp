#include <Windows.h>
// Windowsアプリでのエントリーポイント
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE,_In_ LPSTR,_In_ int){
	// 出力ウィンドウへの文字出力
	OutputDebugStringA("Hellow DirectX!\n");

	return 0;
}
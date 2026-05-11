#include "Logger.h"
#include <Windows.h>
#include <format>			// フォーマットの自動指定をしてくれる(+安全策)
#include <filesystem>		// ファイルを読んだり書いたりするためのライブラリ
#include <chrono>			// 時間を扱うためのライブラリ

/*-------------------- wstring->stringの変換関数 --------------------*/
std::string ConvertString(const std::wstring& str) {
	if (str.empty()) {
		return std::string();
	}

	auto sizeNeeded = WideCharToMultiByte(CP_UTF8,0,str.data(),static_cast<int>(str.size()),NULL,0,NULL,NULL);
	if (sizeNeeded == 0) {
		return std::string();
	}
	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8,0,str.data(),static_cast<int>(str.size()),result.data(),sizeNeeded,NULL,NULL);
	return result;
}

/*-------------------- 出力ウィンドウに文字列を表示できる様にする関数 --------------------*/
void Log(const std::string& message) {
	OutputDebugStringA(message.c_str());
}
void Log(const std::wstring& message) {
	Log(ConvertString(message));
}

/*-------------------- Logファイルを出力できる様にする関数 --------------------*/
void Log(std::ostream& os, const std::string& message) {
	os << message << std::endl;
	OutputDebugStringA(message.c_str());
}

/*-------------------- Logファイル作成 --------------------*/
std::ofstream CreateLogFile() {
	// Logのディレクトリを用意
	std::filesystem::create_directories("logs");
	// 現在時刻を取得
	auto now = std::chrono::system_clock::now();
	// コンマ何秒かはいらないので、秒単位に変換
	auto nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
	// 日本時刻に変換
	std::chrono::zoned_time localTime{ std::chrono::current_zone(),nowSeconds };
	// formatを使って年月日_時分秒の文字列を作る
	std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);
	// 時刻を使ってログファイルの名前を作る
	std::string filePath = std::string("logs/") + dateString + ".log";
	// ファイルを作って書き込み準備
	std::ofstream logStream(filePath);

	return logStream;
}
#pragma once
// ログ出力用の関数をまとめたヘッダファイル(クラスじゃないぞ)
#include <string>
#include <fstream>

// wstring->stringの変換関数
std::string ConvertString(const std::wstring& str);

// 出力ウィンドウに文字列を表示できる様にする関数
void Log(const std::string& message);
void Log(const std::wstring& message);

// logファイル出力用
void Log(std::ostream& os, const std::string& message);

// logファイル作成
std::ofstream CreateLogFile();
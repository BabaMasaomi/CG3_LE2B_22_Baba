#pragma once
#include <string>
#include <fstream>

std::string ConvertString(const std::wstring& str);

// OutputDebugString用
void Log(const std::string& message);
void Log(const std::wstring& message);

// logファイル出力用
void Log(std::ostream& os, const std::string& message);

// logファイル作成
std::ofstream CreateLogFile();
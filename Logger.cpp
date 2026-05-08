#include "Logger.h"
#include <Windows.h>
#include <format>
#include <filesystem>
#include <chrono>

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

void Log(const std::string& message) {
	OutputDebugStringA(message.c_str());
}

void Log(const std::wstring& message) {
	Log(ConvertString(message));
}

void Log(std::ostream& os, const std::string& message) {
	os << message << std::endl;
	OutputDebugStringA(message.c_str());
}

std::ofstream CreateLogFile() {
	std::filesystem::create_directories("logs");

	auto now = std::chrono::system_clock::now();

	auto nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);

	std::chrono::zoned_time localTime{ std::chrono::current_zone(),nowSeconds };

	std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);

	std::string filePath = std::string("logs/") + dateString + ".log";

	std::ofstream logStream(filePath);

	return logStream;
}
#pragma once
// ログ出力用の関数をまとめたヘッダファイル(クラスじゃないぞ)
#include <string>
#include <fstream>

/*==================== メンバ関数の宣言 ====================*/
/// <summary>
/// ログを出力する
/// </summary>
/// <param name="message">出力するメッセージ</param>
void Log(const std::string& message);
/// <summary>
/// ワイド文字列のログを出力する
/// </summary>
/// <param name="message">出力するメッセージ</param>
void Log(const std::wstring& message);

/// <summary>
/// 指定の出力ストリームにログを出力する
/// </summary>
/// <param name="os">出力ストリーム</param>
/// <param name="message">出力するメッセージ</param>
void Log(std::ostream& os, const std::string& message);

/// <summary>
/// ログファイルを作成する
/// </summary>
/// <returns>作成されたログファイルのofstreamオブジェクト</returns>
std::ofstream CreateLogFile();
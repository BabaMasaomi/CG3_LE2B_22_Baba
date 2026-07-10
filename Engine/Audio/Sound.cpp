#include "Sound.h"
#include <cassert>

namespace Engine {

	/*-------------------- 初期化 --------------------*/
	void Sound::Initialize() {
		HRESULT result;

		// XAudioのインスタンスを生成
		result = XAudio2Create(xAudio2_.GetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR);
		assert(SUCCEEDED(result));

		// マスターボイスを生成
		result = xAudio2_.Get()->CreateMasteringVoice(&masterVoice_);
		assert(SUCCEEDED(result));
	}

	/*-------------------- 音声データの読み込み --------------------*/
	SoundData Sound::SoundLoadWave(const char* filename) {
		// 1.ファイルオープン 
		// ファイル入力ストリームのインスタンス
		std::ifstream file;
		// .wavファイルをバイナリモードで開く
		file.open(filename, std::ios_base::binary);
		// ファイルオープン失敗を検出する
		assert(file.is_open());

		// 2.wavデータ読み込み 
		// RIFFヘッダーの読み込み
		RiffHeader riff;
		file.read((char*)&riff, sizeof(riff));
		// ファイルがRIFFかチェック
		if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
			assert(0);
		}
		// タイプがWAVEかチェック
		if (strncmp(riff.type, "WAVE", 4) != 0) {
			assert(0);
		}

		// Formatチャンクの読み込み
		FormatChunk format = {};
		// チャンクヘッダーの確認
		file.read((char*)&format, sizeof(ChunkHeader));
		if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
			assert(0);
		}

		// チャンク本体の読み込み
		assert(format.chunk.size <= sizeof(format.fmt));
		file.read((char*)&format.fmt, format.chunk.size);

		// Dataチャンクの読み込み
		ChunkHeader data;
		file.read((char*)&data, sizeof(data));
		// JUNKチャンクを検出した場合
		if (strncmp(data.id, "JUNK", 4) == 0) {
			// 読み取り位置をJUNKチャンクの終わりまで進める
			file.seekg(data.size, std::ios_base::cur);
			// 再度読み込み
			file.read((char*)&data, sizeof(data));
		}
		// Dataチャンクかチェック
		if (strncmp(data.id, "data", 4) != 0) {
			assert(0);
		}

		// returnする為の音声データ
		SoundData soundData = {};

		// Dataチャンクのデータ部(波形データ)の読み込み
		soundData.buffer.resize(data.size);
		file.read(reinterpret_cast<char*>(soundData.buffer.data()), data.size);

		// 3.ファイルクローズ
		// Waveファイルを閉じる
		file.close();

		// 4.読み込んだ音声データをretun		
		soundData.wfex = format.fmt;

		return soundData;
	}

	/*-------------------- サウンドの再生 --------------------*/
	void Sound::SoundPlayWave(const SoundData& soundData) {
		HRESULT result;

		// 波形フォーマットを元にSourceVoiceの生成
		IXAudio2SourceVoice* pSourceVoice = nullptr;
		result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
		assert(SUCCEEDED(result));

		// 再生する波形データの設定
		XAUDIO2_BUFFER buf{};
		buf.pAudioData = soundData.buffer.data();
		buf.AudioBytes =static_cast<UINT32>(soundData.buffer.size());
		buf.Flags = XAUDIO2_END_OF_STREAM;

		// 波形データの再生
		result = pSourceVoice->SubmitSourceBuffer(&buf);
		result = pSourceVoice->Start();
	}

	/*-------------------- 終了処理 --------------------*/
	void Sound::Finalize() {
		masterVoice_->DestroyVoice();
		masterVoice_ = nullptr;
	}
}
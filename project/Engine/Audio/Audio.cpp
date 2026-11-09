#include "Audio.h"
#include <algorithm>
#include <cassert>
#include <cstring>

namespace Engine {

	/*-------------------- 初期化 --------------------*/
	void Audio::Initialize() {
		HRESULT result;

		// XAudioのインスタンスを生成
		result = XAudio2Create(xAudio2_.GetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR);
		assert(SUCCEEDED(result));

		// マスターボイスを生成
		result = xAudio2_.Get()->CreateMasteringVoice(&masterVoice_);
		assert(SUCCEEDED(result));
	}

	/*-------------------- 音声データの読み込み --------------------*/
	SoundData Audio::AudioLoadWave(const char* filename) {
		// WAVファイルをバイナリモードで開く
		std::ifstream file(filename, std::ios_base::binary);
		assert(file.is_open());

		// RIFFヘッダーを読み込む
		RiffHeader riff{};
		file.read(reinterpret_cast<char*>(&riff), sizeof(RiffHeader));
		assert(file.good());

		// RIFF形式か確認
		if (std::strncmp(riff.chunk.id, "RIFF", 4) != 0) {
			assert(false);
		}

		// WAVE形式か確認
		if (std::strncmp(riff.type, "WAVE", 4) != 0) {
			assert(false);
		}

		// サウンドデータ
		SoundData soundData{};
		// チャンクが見つかったか
		bool foundFormat = false;
		bool foundData = false;

		// fmtチャンクとdataチャンクが見つかるまで調べる
		while (file && (!foundFormat || !foundData)) {
			ChunkHeader chunk{};

			// チャンクヘッダーを読み込む
			file.read(reinterpret_cast<char*>(&chunk), sizeof(ChunkHeader));
			if (!file) {
				break;
			}

			// fmtチャンク
			if (std::strncmp(chunk.id, "fmt ", 4) == 0) {
				// 現在のSoundDataはWAVEFORMATEXまでを保存する設計
				assert(chunk.size <= sizeof(WAVEFORMATEX));

				// 念のためゼロクリア
				soundData.wfex = {};

				// フォーマットを読み込む
				file.read(reinterpret_cast<char*>(&soundData.wfex), chunk.size);
				assert(file.good());

				foundFormat = true;

			} else if (std::strncmp(chunk.id, "data", 4) == 0) {
				// dataチャンク
				// 波形データ用バッファを確保
				soundData.buffer.resize(static_cast<size_t>(chunk.size));
				// 波形データを読み込む
				file.read(reinterpret_cast<char*>(soundData.buffer.data()), chunk.size);
				assert(file.good());

				foundData = true;

			} else {
				// LIST、JUNK、factなど、使用しないチャンク
				// チャンク本体を読み飛ばす
				file.seekg(static_cast<std::streamoff>(chunk.size), std::ios_base::cur);
			}

			// RIFFチャンクは偶数バイト境界に配置される
			// サイズが奇数ならパディング1バイトを読み飛ばす
			if ((chunk.size & 1) != 0) {
				file.seekg(1, std::ios_base::cur);
			}
		}

		// 必要なチャンクが見つかったか確認
		assert(foundFormat);
		assert(foundData);
		assert(!soundData.buffer.empty());

		file.close();

		return soundData;
	}

	/*-------------------- サウンドの再生 --------------------*/
	void Audio::AudioPlayWave(const SoundData& soundData) {
		// 音声が無ければ停止
		assert(xAudio2_ != nullptr);
		assert(!soundData.buffer.empty());

		// 重なりを許可しない場合、現在鳴っている音をすべて停止する
		if (!allowOverlap_) {
			StopAllSounds();
		}

		// sourceVoice
		IXAudio2SourceVoice* sourceVoice = nullptr;

		// 波形フォーマットを元にSourceVoiceの生成
		HRESULT result = xAudio2_->CreateSourceVoice(&sourceVoice, &soundData.wfex);
		// SourceVoiceの生成ができていなければ止める
		if (FAILED(result)) {
			assert(false);
			return;
		}

		// 音声ごとの波形フォーマットでSourceVoiceを作成
		XAUDIO2_BUFFER buffer{};
		buffer.pAudioData = soundData.buffer.data();
		buffer.AudioBytes = static_cast<UINT32>(soundData.buffer.size());
		buffer.Flags = XAUDIO2_END_OF_STREAM;

		// 波形データの再生
		result = sourceVoice->SubmitSourceBuffer(&buffer);
		// SourceVoiceを破棄
		if (FAILED(result)) {
			sourceVoice->DestroyVoice();
			assert(false);
			return;
		}

		// SourceVoiceの再生を開始
		result = sourceVoice->Start();
		// SourceVoiceを破棄
		if (FAILED(result)) {
			sourceVoice->DestroyVoice();
			assert(false);
			return;
		}

		// 再生中Voiceとして保存
		sourceVoices_.push_back(sourceVoice);
	}

	/*-------------------- 再生終了したSourceVoiceを破棄 --------------------*/
	void Audio::Update() {
		for (auto iterator = sourceVoices_.begin();
			iterator != sourceVoices_.end();) {
			// sourceVoiceを
			IXAudio2SourceVoice* sourceVoice = *iterator;

			// Voiceのstate
			XAUDIO2_VOICE_STATE state{};
			sourceVoice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);

			// 再生するバッファが残っていなければ終了
			if (state.BuffersQueued == 0) {
				// 再生終了
				sourceVoice->DestroyVoice();
				// イテレーターを戻す
				iterator = sourceVoices_.erase(iterator);

			} else {
				// イテレーターを増やす
				++iterator;
			}
		}
	}

	/*-------------------- すべての音声を停止 --------------------*/
	void Audio::StopAllSounds() {
		for (IXAudio2SourceVoice* sourceVoice : sourceVoices_) {
			if (sourceVoice == nullptr) {
				continue;
			}
			// SourceVoiceを停止、破棄する
			sourceVoice->Stop();
			sourceVoice->FlushSourceBuffers();
			sourceVoice->DestroyVoice();
		}

		// SourceVoiceをクリアする
		sourceVoices_.clear();
	}

	/*-------------------- 音声の重なりを設定 --------------------*/
	void Audio::SetAllowOverlap(bool allowOverlap) {
		// 音声を変更していないならスキップ
		if (allowOverlap_ == allowOverlap) {
			return;
		}

		allowOverlap_ = allowOverlap;

		// 重なり禁止へ切り替えた時点で、現在鳴っている音をすべて停止する
		if (!allowOverlap_) {
			StopAllSounds();
		}
	}

	/*-------------------- 終了処理 --------------------*/
	void Audio::Finalize() {
		// 再生中の音声をすべて停止
		StopAllSounds();
		// マスターボイスを破棄
		if (masterVoice_ != nullptr) {
			masterVoice_->DestroyVoice();
			masterVoice_ = nullptr;
		}
		// リセットする
		xAudio2_.Reset();
	}
}
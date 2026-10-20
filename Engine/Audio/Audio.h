#pragma once
#include <xaudio2.h>
#include <fstream>
#include <wrl.h>
#include <vector>

#pragma comment(lib,"xaudio2.lib")

namespace Engine {

	// チャンクヘッダ
	struct ChunkHeader {
		char id[4];		// チャンク毎のID
		uint32_t size;	// チャンクサイズ
	};

	// RIFFヘッダチャンク
	struct RiffHeader {
		ChunkHeader chunk;	// RIFF
		char type[4];		// WAVE
	};

	// FMTチャンク
	struct FormatChunk {
		ChunkHeader chunk;	// fmt
		WAVEFORMATEX fmt;	// 波形フォーマット
	};

	// サウンドデータ
	struct SoundData {
		WAVEFORMATEX wfex{};		// 波形フォーマット
		std::vector<BYTE> buffer;	// バッファのデータ
	};

	/// <summary>
	/// サウンドの管理クラス
	/// </summary>
	class Audio {
	public:
		/*==================== メンバ関数の宣言 ====================*/
		/// <summary>
		/// サウンドの初期化
		/// </summary>
		void Initialize();

		/// <summary>
		/// WAVEファイルのロード
		/// </summary>
		/// <param name="filename">ファイル名</param>
		/// <returns>ロードしたサウンドデータ</returns>
		SoundData AudioLoadWave(const char* filename);

		/// <summary>
		/// WAVEファイルの再生
		/// </summary>
		/// <param name="soundData">再生するサウンドデータ</param>
		void AudioPlayWave(const SoundData& soundData);

		/// <summary>
		/// 再生終了したSourceVoiceを破棄
		/// </summary>
		void Update();

		/// <summary>
		/// すべての音声を停止
		/// </summary>
		void StopAllSounds();

		/// <summary>
		/// 音声の重なりを設定
		/// </summary>
		/// <param name="allowOverlap">重複再生の許可・禁止</param>
		void SetAllowOverlap(bool allowOverlap);

		/// <summary>
		/// サウンドの終了処理
		/// </summary>
		void Finalize();

		/*==================== アクセッサ ====================*/
		// ゲッター
		bool GetAllowOverlap() const { return allowOverlap_; }

	private:
		/*==================== メンバ変数 ====================*/
		// XAudio2のインスタンス
		Microsoft::WRL::ComPtr<IXAudio2> xAudio2_ = nullptr;
		// マスターボイス
		IXAudio2MasteringVoice* masterVoice_ = nullptr;
		// 再生中のSourceVoice
		std::vector<IXAudio2SourceVoice*> sourceVoices_;

		// true：複数音声の同時再生を許可
		// false：新しい音を鳴らす前に既存の音を停止
		bool allowOverlap_ = false;
	};
}
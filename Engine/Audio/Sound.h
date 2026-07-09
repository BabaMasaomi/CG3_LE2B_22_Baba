#pragma once
#include <xaudio2.h>
#include <fstream>
#include <wrl.h>

#pragma comment(lib,"xaudio2.lib")

namespace Engine {

	// チャンクヘッダ
	struct ChunkHeader {
		char id[4];		// チャンク毎のID
		int32_t size;	// チャンクサイズ
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

	struct SoundData {
		WAVEFORMATEX wfex;			// 波形フォーマット
		BYTE* pBuffer = nullptr;	// バッファの先頭アドレス
		uint32_t bufferSize = 0;	// バッファのサイズ
	};

	/// <summary>
	/// サウンドの管理クラス
	/// </summary>
	class Sound {
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
		SoundData SoundLoadWave(const char* filename);

		/// <summary>
		/// サウンドデータのアンロード
		/// </summary>
		/// <param name="soundData">アンロードするサウンドデータ</param>
		void SoundUnload(SoundData* soundData);

		/// <summary>
		/// WAVEファイルの再生
		/// </summary>
		/// <param name="soundData">再生するサウンドデータ</param>
		void SoundPlayWave(const SoundData& soundData);

		/// <summary>
		/// サウンドの終了処理
		/// </summary>
		void Finalize();

	private:
		/*==================== メンバ変数 ====================*/
		// XAudio2のインスタンス
		Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
		// マスターボイス
		IXAudio2MasteringVoice* masterVoice_;
	};
}
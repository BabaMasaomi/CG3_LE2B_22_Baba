#pragma once
#include <xaudio2.h>
#include <fstream>
#include <wrl.h>

#pragma comment(lib,"xaudio2.lib")

// チャンクヘッダ
struct ChunkHeader{
	char id[4];		// チャンク毎のID
	int32_t size;	// チャンクサイズ
};

// RIFFヘッダチャンク
struct RiffHeader{
	ChunkHeader chunk;	// RIFF
	char type[4];		// WAVE
};

// FMTチャンク
struct FormatChunk{
	ChunkHeader chunk;	// fmt
	WAVEFORMATEX fmt;	// 波形フォーマット
};

struct SoundData {
	WAVEFORMATEX wfex;			// 波形フォーマット
	BYTE* pBuffer = nullptr;	// バッファの先頭アドレス
	uint32_t bufferSize = 0;	// バッファのサイズ
};

class Sound{
public:
	void Initialize();
	SoundData SoundLoadWave(const char* filename);
	void SoundUnload(SoundData* soundData);
	void SoundPlayeWave(const SoundData& soundData);
	void Finalize();

private:
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
	IXAudio2MasteringVoice* masterVoice_;
};
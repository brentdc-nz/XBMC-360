#pragma once

#include "CachingCodec.h"
// TODO BRENT - port MusicInfoTagLoaderMP3 for seek/replaygain/ID3v2
// #include "music/tags/MusicInfoTagLoaderMP3.h"
#include "MadMp3Decoder.h"

class MP3Codec : public CachingCodec
{
public:
	MP3Codec();
	virtual ~MP3Codec();

	virtual bool Init(const CStdString &strFile, unsigned int filecache);
	virtual void DeInit();
	virtual bool CanSeek();
	virtual __int64 Seek(__int64 iSeekTime);
	virtual int ReadPCM(BYTE *pBuffer, int size, int *actualsize);
	virtual int ReadSamples(float *pBuffer, int numsamples, int *actualsamples);
	virtual bool CanInit();
	virtual bool SkipNext();
	virtual bool HasFloatData() const { return m_BitsPerSampleInternal == 32; };

private:
	void OnFileReaderClearEvent();
	void FlushDecoder();
	int Read(int size, bool init = false);

	// Decoding variables
	__int64 m_lastByteOffset;
	bool    m_eof;
	MadMp3Decoder* m_pDecoder;    // handle to the codec.
	bool    m_Decoding;
	bool    m_CallAgainWithSameBuffer;
	int     m_readRetries;

	// Input buffer to read our mp3 data into
	BYTE*         m_InputBuffer;
	unsigned int  m_InputBufferSize;
	unsigned int  m_InputBufferPos;

	// Output buffer.  We require this, as mp3 decoding means keeping at least 2 frames (1152 * 2 samples)
	// of data in order to remove that data at the end as it may be surplus to requirements.
	BYTE*         m_OutputBuffer;
	unsigned int  m_OutputBufferSize;
	unsigned int  m_OutputBufferPos;    // position in our buffer

	unsigned int m_Formatdata[8];

	// Seeking helpers
	// TODO BRENT - port CVBRMP3SeekHelper
	// MUSIC_INFO::CVBRMP3SeekHelper m_seekInfo;

	// Gapless playback
	bool m_IgnoreFirst;     // Ignore first samples if this is true (for gapless playback)
	bool m_IgnoreLast;      // Ignore first samples if this is true (for gapless playback)
	int m_IgnoredBytes;     // amount of samples ignored thus far

	int m_BitsPerSampleInternal;
};

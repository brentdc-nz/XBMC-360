#ifndef H_CDVDPLAYERAUDIO
#define H_CDVDPLAYERAUDIO

#include "..\..\utils\Thread.h"
#include "DVDMessageQueue.h"
#include "DVDCodecs\DVDAudioCodec.h"
#include "DVDClock.h"
#include "DVDStreamInfo.h"

#include "DVDAudioDevice.h"

#include <queue>

#define DECODE_FLAG_DROP    1
#define DECODE_FLAG_RESYNC  2
#define DECODE_FLAG_ERROR   4
#define DECODE_FLAG_ABORT   8

typedef struct stDVDAudioFrame
{
	BYTE* data;
	__int64 pts;
	unsigned int duration;  
	unsigned int size;
} DVDAudioFrame;

class CDVDPlayerAudio : public CThread
{
public:
	CDVDPlayerAudio(CDVDClock* pClock);
	virtual ~CDVDPlayerAudio();
	
	bool OpenStream( CDVDStreamInfo &hints );
	void CloseStream(bool bWaitForBuffers);

	void SetSpeed(int speed);
	void Flush();

	bool AcceptsData()                                    { return !m_messageQueue.IsFull(); }
	void SendMessage(CDVDMsg* pMsg)                       { m_messageQueue.Put(pMsg); }

	// Holds stream information for current playing stream
	CDVDStreamInfo m_streaminfo;

	CDVDMessageQueue m_messageQueue;

	__int64 GetCurrentPts();

	bool IsStalled()										{ return m_Stalled;  }

protected:
	virtual void OnStartup();
	virtual void OnExit();
	virtual void Process();

	bool InitializeOutputDevice();
	int DecodeFrame(DVDAudioFrame &audioframe, bool bDropPacket);

	// Tries to open a decoder for the given data. 
	bool OpenDecoder(CDVDStreamInfo &hint);

	bool m_bInitializedOutputDevice;
	__int64 m_audioClock;

	// For audio decoding
	CDVDDemux::DemuxPacket* pAudioPacket;
	BYTE* audio_pkt_data; // current audio packet
	int audio_pkt_size; // current audio packet size

	CDVDAudio m_dvdAudio; // audio output device
	CDVDClock* m_pClock; // dvd master clock
	CDVDAudioCodec* m_pAudioCodec; // audio codec

	int m_speed;  // wanted playback speed. if playback speed!=DVD_PLAYSPEED_NORMAL, don't sync clock as it will loose track of position after seek

	typedef struct {__int64 pts; __int64 timestamp;} TPTSItem;
	TPTSItem m_currentPTSItem;
	std::queue<TPTSItem> m_quePTSQueue;

	void AddPTSQueue(__int64 pts, __int64 delay);
	void FlushPTSQueue();

	bool m_Stalled;
	CRITICAL_SECTION m_critCodecSection;
};

#endif //H_CDVDPLAYERAUDIO
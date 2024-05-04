#ifndef H_CDVDPLAYERAUDIO
#define H_CDVDPLAYERAUDIO

#include "Utils/Thread.h"
#include "DVDClock.h"
#include "DVDMessageQueue.h"
#include "DVDStreamInfo.h"
#include "DVDCodecs\Audio\DVDAudioCodec.h"
#include "Utils\BitstreamStats.h"
#include "DVDAudio.h"

#include <queue>

class CDVDPlayer;
class CDVDAudioCodec;
class IAudioCallback;
class CDVDAudioCodec;

enum CodecID;

#define DECODE_FLAG_DROP    1
#define DECODE_FLAG_RESYNC  2
#define DECODE_FLAG_ERROR   4
#define DECODE_FLAG_ABORT   8
#define DECODE_FLAG_TIMEOUT 16

typedef struct stDVDAudioFrame
{
	BYTE* data;
	double pts;
	double duration;
	unsigned int size;

	int channels;
	int bits_per_sample;
	int sample_rate;
	bool passthrough;
} DVDAudioFrame;

class CPTSOutputQueue
{
private:
	typedef struct {double pts; double timestamp; double duration;} TPTSItem;
	TPTSItem m_current;
	std::queue<TPTSItem> m_queue;
	CCriticalSection m_sync;

public:
	CPTSOutputQueue();
	void Add(double pts, double delay, double duration);
	void Flush();
	double Current();
};

class CPTSInputQueue
{
private:
	typedef std::list<std::pair<__int64, double> >::iterator IT;
	std::list<std::pair<__int64, double> > m_list;
	CCriticalSection m_sync;

public:
	void   Add(__int64 bytes, double pts);
	double Get(__int64 bytes, bool consume);
	void   Flush();
};

class CDVDPlayerAudio : public CThread
{
public:
	CDVDPlayerAudio(CDVDClock* pClock, CDVDMessageQueue& parent);
	virtual ~CDVDPlayerAudio();

	bool OpenStream(CDVDStreamInfo &hints);
	void OpenStream(CDVDStreamInfo &hints, CDVDAudioCodec* codec);
	void CloseStream(bool bWaitForBuffers);
	void SetSpeed(int speed);
	void Flush();

	bool AcceptsData()                                    { return !m_messageQueue.IsFull(); }
	void SendMessage(CDVDMsg* pMsg, int priority = 0)     { m_messageQueue.Put(pMsg, priority); }
	bool IsStalled()                                      { return m_stalled;  }
	
	void SetVolume(long nVolume)                          { m_dvdAudio.SetVolume(nVolume); }

	std::string GetPlayerInfo();
	int GetAudioBitrate();

	// Holds stream information for current playing stream
	CDVDStreamInfo m_streaminfo;

	CDVDMessageQueue m_messageQueue;
	CDVDMessageQueue& m_messageParent;
	CPTSOutputQueue m_ptsOutput;
	CPTSInputQueue  m_ptsInput;

	double GetCurrentPts()                                { return m_ptsOutput.Current(); }

protected:
	virtual void OnStartup();
	virtual void OnExit();
	virtual void Process();

	int DecodeFrame(DVDAudioFrame &audioframe, bool bDropPacket);

	double m_audioClock;

	// Data for audio decoding
	struct
	{
		CDVDMsgDemuxerPacket*  msg;
		BYTE*                  data;
		int                    size;
		double                 dts;

		void Attach(CDVDMsgDemuxerPacket* msg2)
		{
			msg = msg2;
			msg->Acquire();
			DemuxPacket* p = msg->GetPacket();
			data = p->pData;
			size = p->iSize;
			dts = p->dts;
		}
		void Release()
		{
			if(msg) msg->Release();
			msg  = NULL;
			data = NULL;
			size = 0;
			dts  = DVD_NOPTS_VALUE;
	   }
	} m_decode;

	CDVDAudio m_dvdAudio; // Audio output device
	CDVDClock* m_pClock; // Master clock
	CDVDAudioCodec* m_pAudioCodec; // Audio codec
	BitstreamStats m_audioStats;

	int     m_speed;
	double  m_droptime;
	bool    m_stalled;
	bool    m_started;
	double  m_duration; // Last packets duration

	bool OutputPacket(DVDAudioFrame &audioframe);
	double m_error; // Last average error
	int64_t m_errortime; // Timestamp of last time we measured
	int64_t m_freq;
	void   HandleSyncError(double duration);
	double m_errorbuff; // Place to store average errors
	int    m_errorcount; // Number of errors stored
	bool   m_syncclock;
};

#endif //H_CDVDPLAYERAUDIO
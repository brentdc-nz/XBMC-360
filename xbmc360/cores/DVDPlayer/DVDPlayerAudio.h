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

class CPTSInputQueue
{
private:
	typedef std::list<std::pair<int64_t, double> >::iterator IT;
	std::list<std::pair<int64_t, double> > m_list;
	CCriticalSection m_sync;

public:
	void   Add(int64_t bytes, double pts);
	double Get(int64_t bytes, bool consume);
	void   Flush();
};

class CDVDPlayerAudio : public CThread
{
public:
	CDVDPlayerAudio(CDVDClock* pClock, CDVDMessageQueue& parent);
	virtual ~CDVDPlayerAudio();

	void RegisterAudioCallback(IAudioCallback* pCallback) { m_dvdAudio.RegisterAudioCallback(pCallback); }
	void UnRegisterAudioCallback()                        { m_dvdAudio.UnRegisterAudioCallback(); }
	void DoAudioWork()                                    { m_dvdAudio.DoAudioWork(); }

	bool OpenStream(CDVDStreamInfo &hints);
	void OpenStream(CDVDStreamInfo &hints, CDVDAudioCodec* codec);
	void CloseStream(bool bWaitForBuffers);

	void SetSpeed(int speed);
	void Flush();

	// Waits until all available data has been rendered
	void WaitForBuffers();
	bool AcceptsData() const                              { return !m_messageQueue.IsFull(); }
	bool HasData() const                                  { return m_messageQueue.GetDataSize() > 0; }
	int  GetLevel() const                                 { return m_messageQueue.GetLevel(); }
	bool IsInited() const                                 { return m_messageQueue.IsInited(); }
	void SendMessage(CDVDMsg* pMsg, int priority = 0)     { m_messageQueue.Put(pMsg, priority); }

	void SetVolume(long nVolume)                          { m_dvdAudio.SetVolume(nVolume); }
//	void SetDynamicRangeCompression(long drc)             { m_dvdAudio.SetDynamicRangeCompression(drc); }

	CDVDMessageQueue m_messageQueue;
	CDVDMessageQueue& m_messageParent;

	std::string GetPlayerInfo();
	int GetAudioBitrate();

	// Holds stream information for current playing stream
	CDVDStreamInfo m_streaminfo;

	CPTSOutputQueue m_ptsOutput;
	CPTSInputQueue  m_ptsInput;

	bool IsStalled()                                  { return m_stalled;  }

	double GetCurrentPts()                                { return m_dvdAudio.GetPlayingPts(); }

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
	bool    m_silence;
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
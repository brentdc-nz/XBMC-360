#ifndef H_CDVDPLAYER
#define H_CDVDPLAYER

#include "..\IPlayer.h"

#include "..\..\utils\Thread.h"
#include "DVDDemuxers\DVDDemux.h"

#include "DVDStreamInfo.h"

#include "DVDPlayerAudio.h"
#include "DVDPlayerVideo.h"

using namespace std;

typedef struct DVDInfo
{
	int iFlagSentStart;
}
DVDInfo;

typedef struct SCurrentStream
{
	int              id;     // demuxerid of current playing stream
	unsigned __int64 dts;    // last dts from demuxer, used to find disncontinuities
	CDVDStreamInfo      hint;   // stream hints, used to notice stream changes
	void*            stream; // pointer or integer, identifying stream playing. if it changes stream changed
} SCurrentStream;

#define DVDPLAYER_AUDIO 1
#define DVDPLAYER_VIDEO 2

class CDVDPlayer : public IPlayer, public CThread
{
public:
	CDVDPlayer(IPlayerCallback& callback);
	~CDVDPlayer();

	virtual bool OpenFile(const string& strFile);
	virtual bool CloseFile();
	virtual bool IsPlaying() const;
	virtual bool IsPaused() const;
	virtual bool HasVideo();
	virtual bool HasAudio();

	virtual void OnStartup();
	virtual void Process();
	virtual void OnExit();

	virtual void SeekTime(__int64 iTime);
	virtual void GetVideoInfo(CStdString& strVideoInfo);
	virtual void GetAudioInfo(CStdString& strAudioInfo);
	virtual void GetGeneralInfo(CStdString& strGeneralInfo);
	virtual void Pause();

	virtual bool OnAction(const CAction &action);

	virtual void SeekPercentage(float iPercent);
	virtual void Seek(bool bPlus, bool bLargeStep);
	virtual __int64 GetTime();
	virtual int GetTotalTime();
	virtual float GetPercentage();
	virtual __int64 GetTotalTimeInMsec();
	virtual void FlushBuffers();

	bool OpenDefaultAudioStream();
	bool OpenAudioStream(int iStream);
	bool OpenVideoStream(int iStream);
	bool CloseAudioStream(bool bWaitForBuffers);
	bool CloseVideoStream(bool bWaitForBuffers);

	bool m_bAbortRequest;

	CDemuxStreamVideo* m_pCurrentDemuxStreamVideo;
	CDemuxStreamAudio* m_pCurrentDemuxStreamAudio;

	CDVDPlayerAudio m_dvdPlayerAudio;
	CDVDPlayerVideo m_dvdPlayerVideo;

private:
	void LockStreams()				{ EnterCriticalSection(&m_critStreamSection); }
	void UnlockStreams()			{ LeaveCriticalSection(&m_critStreamSection); }
	
	void ProcessAudioData(CDemuxStream* pStream, CDVDDemux::DemuxPacket* pPacket);
    void ProcessVideoData(CDemuxStream* pStream, CDVDDemux::DemuxPacket* pPacket);
	
	/**
	* one of the DVD_PLAYSPEED defines
	*/
	void SetPlaySpeed(int iSpeed);

	int GetPlaySpeed()				{ return m_playSpeed; }

	void HandleMessages();
	void SyncronizePlayers(DWORD sources);
	void CheckContinuity(CDVDDemux::DemuxPacket* pPacket, unsigned int source);

	bool m_bDontSkipNextFrame;
	
	int m_playSpeed;

	unsigned int m_packetcount; // packet count from demuxer, may wrap around. used during startup

	string		m_strFilename;

	SCurrentStream m_CurrentAudio;
	SCurrentStream m_CurrentVideo;	

	CDVDDemux*	m_pDemuxer;

	CDVDMessageQueue m_messenger;     // thread messenger, only the dvdplayer.cpp class itself may send message to this!

	CDVDClock m_clock; // master clock

	DVDInfo m_dvd;

	HANDLE m_hReadyEvent;
	CRITICAL_SECTION m_critStreamSection; // need to have this lock when switching streams (audio / video)
};

#endif //H_CDVDPLAYER
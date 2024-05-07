#ifndef H_CDVDPLAYER
#define H_CDVDPLAYER

#include "..\IPlayer.h"
#include "..\..\utils\Thread.h"

#include "IDVDPlayer.h"
#include "DVDClock.h"
#include "DVDInputStreams\DVDInputStream.h"
#include "DVDPlayerVideo.h"
#include "DVDPlayerAudio.h"
#include "DVDStreamInfo.h"
#include "Edl.h"

class CDVDInputStream;
class CDVDPlayer;

#define DVDSTATE_NORMAL           0x00000001 // Normal dvd state
#define DVDSTATE_STILL            0x00000002 // Currently displaying a still frame
#define DVDSTATE_WAIT             0x00000003 // Waiting for demuxer read error

class CCurrentStream
{
public:
	int              id;      // Demuxerid of current playing stream
	int              source;
	double           dts;     // Last dts from demuxer, used to find disncontinuities
	CDVDStreamInfo   hint;    // Stream hints, used to notice stream changes
	void*            stream;  // Pointer or integer, identifying stream playing. if it changes stream changed
	bool             inited;
	bool             started; // Has the player started
	const StreamType type;
	// Stuff to handle starting after seek
	double   startpts;
	CDVDMsg* startsync;

	CCurrentStream(StreamType t) : type(t)
	{
		startsync = NULL;
		Clear();
	}

	void Clear()
	{
		id     = -1;
		source = STREAM_SOURCE_NONE;
		dts    = DVD_NOPTS_VALUE;
		hint.Clear();
		stream = NULL;
		inited = false;
		started = false;

		if(startsync)
			startsync->Release();

		startsync = NULL;
		startpts  = DVD_NOPTS_VALUE;
	}
};

typedef struct DVDInfo
{
	int iFlagSentStart;
}
DVDInfo;

typedef struct
{
	StreamType   type;
	std::string  filename;
	std::string  language;
	std::string  name;
	CDemuxStream::EFlags flags;
	int          source;
	int          id;
} SelectionStream;

class CSelectionStreams
{
	CCriticalSection m_section;
	SelectionStream  m_invalid;
public:
	CSelectionStreams()
	{
		m_invalid.id = -1;
		m_invalid.source = STREAM_SOURCE_NONE;
		m_invalid.type = STREAM_NONE;
	}
	std::vector<SelectionStream> m_Streams;

	int              IndexOf (StreamType type, int source, int id);
	int              IndexOf (StreamType type, CDVDPlayer& p);
	int              Count   (StreamType type) { return IndexOf(type, STREAM_SOURCE_NONE, -1) + 1; }
	SelectionStream& Get     (StreamType type, int index);
	bool             Get     (StreamType type, CDemuxStream::EFlags flag, SelectionStream& out);

	void             Clear   (StreamType type, StreamSource source);
	int              Source  (StreamSource source, std::string filename);

	void             Update  (SelectionStream& s);
	void             Update  (CDVDInputStream* input, CDVDDemux* demuxer);
};

#define DVDPLAYER_AUDIO    1
#define DVDPLAYER_VIDEO    2
#define DVDPLAYER_SUBTITLE 3

class CDVDPlayer : public IPlayer, public CThread, public IDVDPlayer
{
public:
	CDVDPlayer(IPlayerCallback& callback);
	~CDVDPlayer();

	virtual bool OpenFile(const CFileItem& file, const CPlayerOptions &options);
	virtual bool CloseFile();
	virtual bool IsPlaying() const;
	virtual bool IsPaused() const;
	virtual bool HasVideo() const;
	virtual bool HasAudio() const;
	virtual bool CanSeek();
	virtual void OnStartup();
	virtual void Process();
	virtual void OnExit();

	virtual void SeekTime(__int64 iTime);
	virtual void GetVideoInfo(CStdString& strVideoInfo);
	virtual void GetAudioInfo(CStdString& strAudioInfo);
	virtual void GetGeneralInfo(CStdString& strGeneralInfo);
	void CheckContinuity(CCurrentStream& current, DemuxPacket* pPacket);
	virtual void Pause();

	virtual bool OnAction(const CAction &action);

	virtual void SetVolume(long nVolume)                          { m_dvdPlayerAudio.SetVolume(nVolume); }

	virtual void SeekPercentage(float iPercent);
	virtual void Seek(bool bPlus, bool bLargeStep);
	virtual __int64 GetTime();
	virtual int GetTotalTime();
	virtual float GetPercentage();
	virtual __int64 GetTotalTimeInMsec();
	void FlushBuffers(bool queued);
	virtual void SetAVDelay(float fValue = 0.0f);

	bool IsInMenu() const;
	void HandlePlaySpeed();
	void HandleMessages();

	bool OpenDefaultAudioStream();
	bool OpenAudioStream(int iStream, int source);
	bool OpenVideoStream(int iStream, int source);
	bool CloseAudioStream(bool bWaitForBuffers);
	bool CloseVideoStream(bool bWaitForBuffers);

	void ProcessPacket(CDemuxStream* pStream, DemuxPacket* pPacket);
	void ProcessAudioData(CDemuxStream* pStream, DemuxPacket* pPacket);
	void ProcessVideoData(CDemuxStream* pStream, DemuxPacket* pPacket);

	bool m_bAbortRequest;

	enum ECacheState
	{
		CACHESTATE_DONE = 0
		, CACHESTATE_FULL     // Player is filling up the demux queue
		, CACHESTATE_INIT     // Player is waiting for first packet of each stream
		, CACHESTATE_PLAY     // Player is waiting for players to not be stalled
	};

	virtual bool IsCaching() const { return m_caching == CACHESTATE_FULL; }

protected:
	friend class CSelectionStreams;
	void LockStreams()                                            { EnterCriticalSection(&m_critStreamSection); }
	void UnlockStreams()                                          { LeaveCriticalSection(&m_critStreamSection); }

	bool OpenInputStream();
	bool OpenDemuxStream();
	void OpenDefaultStreams();
	void SetPlaySpeed(int iSpeed);
	int GetPlaySpeed()                                             { return m_playSpeed; }   
	void SetCaching(ECacheState state);
	bool CheckStartCaching(CCurrentStream& current);
	void SendPlayerMessage(CDVDMsg* pMsg, unsigned int target);
	bool CheckSceneSkip(CCurrentStream& current);
	bool CheckPlayerInit(CCurrentStream& current, unsigned int source);
	bool ReadPacket(DemuxPacket*& packet, CDemuxStream*& stream);
	bool IsValidStream(CCurrentStream& stream);
	bool IsBetterStream(CCurrentStream& current, CDemuxStream* stream);
	void UpdateApplication(double timeout);
	void UpdatePlayState(double timeout);
	void SynchronizeDemuxer(DWORD timeout);
	void SynchronizePlayers(DWORD sources, double pts = DVD_NOPTS_VALUE);
	void CheckAutoSceneSkip();

	double m_UpdateApplication;

	CCurrentStream m_CurrentAudio;
	CCurrentStream m_CurrentVideo;
	CCurrentStream m_CurrentSubtitle;

	CDVDClock m_clock;                // Master clock
	CDVDMessageQueue m_messenger;     // Thread messenger

	CDVDPlayerVideo m_dvdPlayerVideo; // Video part
	CDVDPlayerAudio m_dvdPlayerAudio; // Audio part

	CSelectionStreams m_SelectionStreams;

	int m_playSpeed;
	struct SSpeedState
	{
		double lastpts;  // Holds last display pts during ff/rw operations
		double lasttime;
	} m_SpeedState;

	std::string m_filename;  // Holds the actual filename
	std::string m_mimetype;  // Hold a hint to what content file contains (mime type)
	ECacheState m_caching;
	CFileItem   m_item;

	int m_errorCount;

	CDVDInputStream* m_pInputStream;  // Input stream for current playing file
	CDVDDemux* m_pDemuxer;            // Demuxer for current playing file
	CDVDDemux* m_pSubtitleDemuxer;

	HANDLE m_hReadyEvent;
	CRITICAL_SECTION m_critStreamSection; // Need to have this lock when switching streams (audio / video)

	CPlayerOptions m_PlayerOptions;
	CEdl m_Edl;

	struct SDVDInfo
	{
		void Clear()
		{
			state                =  DVDSTATE_NORMAL;
			iSelectedSPUStream   = -1;
			iSelectedAudioStream = -1;
			iDVDStillTime        =  0;
			iDVDStillStartTime   =  0;
		}

		int state;                // Current dvdstate
		DWORD iDVDStillTime;      // Total time in ticks we should display the still before continuing
		DWORD iDVDStillStartTime; // Time in ticks when we started the still
		int iSelectedSPUStream;   // mpeg stream id, or -1 if disabled
		int iSelectedAudioStream; // mpeg stream id, or -1 if disabled
	} m_dvd;

	struct SPlayerState
	{
		SPlayerState() { Clear(); }
		void Clear()
		{
			timestamp     = 0;
			time          = 0;
			time_total    = 0;
			time_offset   = 0;
			dts           = DVD_NOPTS_VALUE;
			player_state  = "";
			chapter       = 0;
			chapter_count = 0;
			canrecord     = false;
			recording     = false;
			canseek       = false;
			demux_video   = "";
			demux_audio   = "";
		}

		double timestamp;         // Last time of update
		double time_offset;       // Difference between time and pts

		double time;              // Current playback time
		double time_total;        // Total playback time
		double dts;               // Last known dts

		std::string player_state; // Full player state

		int         chapter;      // Current chapter
		std::string chapter_name; // Name of current chapter
		int         chapter_count;// Number of chapter

		bool canrecord;           // Can input stream record
		bool recording;           // Are we currently recording

		bool canseek;             // pvr: can seek in the current playing item

		std::string demux_video;
		std::string demux_audio;
	} m_State;

	CCriticalSection m_StateSection;

	struct SEdlAutoSkipMarkers
	{
		void Clear()
		{
			cut = -1;
			commbreak_start = -1;
			commbreak_end = -1;
			seek_to_start = false;
			reset = 0;
		}

		void ResetCutMarker(double timeout)
		{
			if(reset != 0 && reset + DVD_MSEC_TO_TIME(timeout) > CDVDClock::GetAbsoluteClock())
				return;
			/*
			* Reset the automatic EDL skip marker for a cut so automatic seeking can happen again if,
			* for example, the initial automatic skip ended up back in the cut due to seeking
			* inaccuracies.
			*/
			cut = -1;

			reset = CDVDClock::GetAbsoluteClock();
		}

		int cut;              // Last automatically skipped EDL cut seek position
		int commbreak_start;  // Start time of the last commercial break automatically skipped
		int commbreak_end;    // End time of the last commercial break automatically skipped
		bool seek_to_start;   // Whether seeking can go back to the start of a previously skipped break
		double reset;         // Last actual reset time
	} m_EdlAutoSkipMarkers;
};

#endif //H_CDVDPLAYER
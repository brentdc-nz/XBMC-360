#ifndef H_CDVDPLAYERVIDEO
#define H_CDVDPLAYERVIDEO

#include "utils\Thread.h"
#include "DVDClock.h"
#include "DVDStreamInfo.h"
#include "DVDMessageQueue.h"
#include "DVDCodecs\Video\DVDVideoCodec.h"
#include "utils\BitstreamStats.h"

#if 1//def HAS_VIDEO_PLAYBACK
#include "../VideoRenderers/RenderManager.h"
#endif

enum CodecID;
class CDemuxStreamVideo;
class CDVDOverlayCodecCC;

#define VIDEO_PICTURE_QUEUE_SIZE 1

class CDVDPlayerVideo : public CThread
{
public:
	CDVDPlayerVideo(CDVDClock* pClock/* 
                 , CDVDOverlayContainer* pOverlayContainer*/
                 , CDVDMessageQueue& parent);
	virtual ~CDVDPlayerVideo();

	bool OpenStream(CDVDStreamInfo &hint);
	void OpenStream(CDVDStreamInfo &hint, CDVDVideoCodec* codec);
	void CloseStream(bool bWaitForBuffers);

	void Flush();
	void SendMessage(CDVDMsg* pMsg, int priority = 0) { m_messageQueue.Put(pMsg, priority); }
	bool AcceptsData()                                { return !m_messageQueue.IsFull(); }
	bool IsStalled()                                  { return m_stalled; }
	double GetCurrentPts()                            { return m_iCurrentPts; }
	void SetDelay(double delay)                       { m_iVideoDelay = delay; }
	double GetDelay()                                 { return m_iVideoDelay; }
	void EnableFullscreen(bool bEnable)               { m_bAllowFullscreen = bEnable; }

	std::string GetPlayerInfo();
	int GetVideoBitrate();

	// Classes
	CDVDMessageQueue m_messageQueue;
	CDVDMessageQueue& m_messageParent;

	void SetSpeed(int iSpeed);

protected:
	virtual void OnStartup();
	virtual void OnExit();
	virtual void Process();

	int OutputPicture(DVDVideoPicture* pPicture, double pts);

	#define EOS_ABORT 1
	#define EOS_DROPPED 2
	#define EOS_VERYLATE 4

	int m_iNrOfPicturesNotToSkip;
	double m_iCurrentPts; // Last pts displayed
	double m_iVideoDelay;
	double m_iSubtitleDelay;
	double m_FlipTimeStamp; // Time stamp of last flippage. used to play at a forced framerate

	int m_iLateFrames;
	int m_iDroppedFrames;
	bool m_bDropFrames;
	int m_iDroppedRequest;

	struct SOutputConfiguration
	{
		unsigned int width;
		unsigned int height;
		unsigned int dwidth;
		unsigned int dheight;
		unsigned int color_matrix : 4;
		unsigned int color_range  : 1;
		float        framerate;
		bool         inited;
	} m_output; // Holds currently configured output

	bool m_bAllowFullscreen;
	bool m_bRenderSubs;
	float m_fForcedAspectRatio;
	float m_fFrameRate;

	// Autosync decides on how much of clock we should use when deciding sleep time
	// the value is the same as 63% timeconstant, ie that the step response of
	// iSleepTime will be at 63% of iClockSleep after autosync frames
	unsigned int m_autosync;

	CDVDClock* m_pClock;

	// Classes
	CDVDStreamInfo m_hints;
	CDVDVideoCodec* m_pVideoCodec;

	BitstreamStats m_videoStats;

	int m_speed;
	double m_droptime;
	double m_dropbase;

	bool m_stalled;
	bool m_started;
	std::string m_codecname;
};

#endif //H_CDVDPLAYERVIDEO
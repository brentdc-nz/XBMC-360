#ifndef H_CDVDPLAYERVIDEO
#define H_CDVDPLAYERVIDEO

#include "utils\Thread.h"

#include "DVDStreamInfo.h"
#include "DVDMessageQueue.h"
#include "DVDCodecs\Video\DVDVideoCodec.h"
#include "utils\BitstreamStats.h"
#include "DVDClock.h"
#if 1//def HAS_VIDEO_PLAYBACK
#include "cores/VideoRenderers/RenderManager.h"
#endif

enum AVCodecID;
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

	void StepFrame();
	void Flush();

	// Waits until all available data has been rendered
	// just waiting for packetqueue should be enough for video
	void WaitForBuffers()                             { m_messageQueue.WaitUntilEmpty(); }
	bool AcceptsData() const                          { return !m_messageQueue.IsFull(); }
	bool HasData() const                              { return m_messageQueue.GetDataSize() > 0; }
	int  GetLevel();
	bool IsInited() const                             { return m_messageQueue.IsInited(); }
	void SendMessage(CDVDMsg* pMsg, int priority = 0) { m_messageQueue.Put(pMsg, priority); }

#ifdef HAS_VIDEO_PLAYBACK
	void Update(bool bPauseDrawing)                   { g_renderManager.Update(bPauseDrawing); }
#else
	void Update(bool bPauseDrawing)                   { }
#endif
 
	void EnableSubtitle(bool bEnable)                 { m_bRenderSubs = bEnable; }
	bool IsSubtitleEnabled()                          { return m_bRenderSubs; }

	void EnableFullscreen(bool bEnable)               { m_bAllowFullscreen = bEnable; }

#ifdef HAS_VIDEO_PLAYBACK
	void GetVideoRect(RECT& SrcRect, RECT& DestRect)  { g_renderManager.GetVideoRect(SrcRect, DestRect); }
	float GetAspectRatio()                            { return g_renderManager.GetAspectRatio(); }
#else
	void GetVideoRect(RECT& SrcRect, RECT& DestRect)  { }
	float GetAspectRatio()                            { return 4.0f / 3.0f; }
#endif

	double GetDelay()                                { return m_iVideoDelay; }
	void SetDelay(double delay)                      { m_iVideoDelay = delay; }

	double GetSubtitleDelay()                                { return m_iSubtitleDelay; }
	void SetSubtitleDelay(double delay)                      { m_iSubtitleDelay = delay; }

	bool IsStalled()                                  { return m_stalled; }
	int GetNrOfDroppedFrames()                        { return m_iDroppedFrames; }

	bool InitializedOutputDevice();
  
	double GetCurrentPts()                           { return m_iCurrentPts; }

	double GetOutputDelay(); /* returns the expected delay, from that a packet is put in queue */
	std::string GetPlayerInfo();
	int GetVideoBitrate();

	void SetSpeed(int iSpeed);

  // Classes
//  CDVDOverlayContainer* m_pOverlayContainer;
  
	CDVDClock* m_pClock;

protected:  
	virtual void OnStartup();
	virtual void OnExit();
	virtual void Process();

#define EOS_ABORT 1
#define EOS_DROPPED 2
#define EOS_VERYLATE 4

	int OutputPicture(DVDVideoPicture* pPicture, double pts);
#ifdef HAS_VIDEO_PLAYBACK
	void ProcessOverlays(DVDVideoPicture* pSource, YV12Image* pDest, double pts);
#endif
	void ProcessVideoUserData(DVDVideoUserData* pVideoUserData, double pts);

	CDVDMessageQueue m_messageQueue;
	CDVDMessageQueue& m_messageParent;

	double m_iCurrentPts; // Last pts displayed
	double m_iVideoDelay;
	double m_iSubtitleDelay;
	double m_FlipTimeStamp; // Time stamp of last flippage. used to play at a forced framerate

	int m_iLateFrames;
	int m_iDroppedFrames;
	int m_iDroppedRequest;

	float m_fFrameRate;

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
  
	int m_iNrOfPicturesNotToSkip;
	int m_speed;

	double m_droptime;
	double m_dropbase;

	bool m_stalled;
	bool m_started;
	std::string m_codecname;

	/* Autosync decides on how much of clock we should use when deciding sleep time */
	/* the value is the same as 63% timeconstant, ie that the step response of */
	/* iSleepTime will be at 63% of iClockSleep after autosync frames */
	unsigned int m_autosync;

	BitstreamStats m_videoStats;
  
	// Classes
	CDVDStreamInfo m_hints;
	CDVDVideoCodec* m_pVideoCodec;
	CDVDOverlayCodecCC* m_pOverlayCodecCC;
  
	DVDVideoPicture* m_pTempOverlayPicture;
 };

#endif //H_CDVDPLAYERVIDEO
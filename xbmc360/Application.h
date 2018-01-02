#ifndef H_CAPPLICATION
#define H_CAPPLICATION

#include "XBApplicationEx.h"
#include "cores\IPlayer.h"
#include "guilib\GUIUserMessage.h"
#include "guilib\IMsgTargetCallback.h"
#include "utils\Stopwatch.h"

class CApplication: public CXBApplicationEX, public IPlayerCallback, public IMsgTargetCallback
{
public:
	CApplication();
	~CApplication();

	virtual bool Create();
	void DelayLoadSkin();
	void CancelDelayLoadSkin();
	void LoadSkin(const CStdString& strSkin);
	void UnloadSkin();

	virtual bool Initialize();
	virtual void Process();
	virtual void FrameMove();
	virtual void Render();
	virtual void Stop();

	bool ProcessGamepad(/*float frameTime*/);

	virtual bool OnMessage(CGUIMessage& message);
	bool NeedRenderFullScreen();
	void RenderFullScreen();
	bool SwitchToFullScreen();

	void StopPlaying();

	virtual void OnPlayBackEnded();
	virtual void OnPlayBackStarted();
	virtual void OnPlayBackStopped();
	virtual void OnQueueNextItem();

	bool PlayFile(const string strFile);
	bool IsPlaying() const;
	bool IsPaused() const;

	bool IsPlayingAudio() const;
	bool IsPlayingVideo() const;

	void ResetScreenSaver();
	bool ResetScreenSaverWindow();
	bool IsInScreenSaver() { return m_bScreenSave; };
	void CheckScreenSaver(); //TODO Make private?

	DWORD m_dwSkinTime;
	IPlayer* m_pPlayer;

protected:
	void FatalErrorHandler(bool InitD3D);
	void  ActivateScreenSaver();

	bool m_bScreenSave;
	CStdString m_screenSaverMode;
	bool m_bInitializing;
	bool m_bPlaybackStarting;

	// Timer information
	CStopWatch m_screenSaverTimer;
};

extern CApplication g_application;

#endif //H_CAPPLICATION
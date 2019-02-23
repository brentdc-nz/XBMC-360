#ifndef H_CAPPLICATION
#define H_CAPPLICATION

#include "XBApplicationEx.h"
#include "cores\IPlayer.h"
#include "guilib\GUIUserMessage.h"
#include "guilib\IMsgTargetCallback.h"
#include "utils\Stopwatch.h"
#include "utils\Splash.h"

#include "guilib\dialogs\GUIDialogSeekBar.h"

class CApplication: public CXBApplicationEX, public IPlayerCallback, public IMsgTargetCallback
{
public:
	CApplication();
	~CApplication();

	virtual bool Create();
	void DelayLoadSkin();
	void CancelDelayLoadSkin();
	void LoadSkin(const CStdString& strSkin);
	void ReloadSkin();
	void UnloadSkin();

	virtual bool Initialize();
	virtual void Process();
	virtual void FrameMove();
	virtual void Render();
	virtual void Stop();

	virtual bool OnMessage(CGUIMessage& message);
	bool NeedRenderFullScreen();
	void RenderFullScreen();
	bool SwitchToFullScreen();
	void StopPlaying();

	// Player callbacks
	virtual void OnPlayBackEnded();
	virtual void OnPlayBackStarted();
	virtual void OnPlayBackStopped();
	virtual void OnQueueNextItem();

	bool PlayFile(const CFileItem& item);
	bool IsPlaying() const;
	bool IsPaused() const;
	int GetPlaySpeed() const;
	void SetPlaySpeed(int iSpeed);

	double GetTime() const;
	double GetTotalTime() const;

	bool IsPlayingAudio() const;
	bool IsPlayingVideo() const;

	void ResetScreenSaver();
	bool ResetScreenSaverWindow();
	bool IsInScreenSaver() { return m_bScreenSave; };
	void CheckScreenSaver();

	DWORD m_dwSkinTime;
	IPlayer* m_pPlayer;

	CGUIDialogSeekBar m_guiDialogSeekBar;

protected:
	bool ProcessGamepad(/*float frameTime*/);
	bool OnKey(CKey& key);
	void FatalErrorHandler(bool InitD3D);
	void ActivateScreenSaver();
	void ProcessSlow();

	bool m_bScreenSave;
	CStdString m_screenSaverMode;
	bool m_bInitializing;
	bool m_bPlaybackStarting;
	int m_iPlaySpeed;

	// Timer information
	CStopWatch m_screenSaverTimer;
	CStopWatch m_slowTimer;

	CSplash* m_splash;
};

extern CApplication g_application;

#endif //H_CAPPLICATION
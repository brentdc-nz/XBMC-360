#ifndef H_CAPPLICATION
#define H_CAPPLICATION

#include "XBApplicationEx.h"
#include "cores\IPlayer.h"
#include "guilib\GUIUserMessages.h"
#include "guilib\IMsgTargetCallback.h"
#include "utils\Stopwatch.h"
#include "utils\Splash.h"
#include "xbox\Network.h"
#include "services\NTPClient\WinSckNTP.h"
#include "services\FTPServer\FTPServer.h"
#include "ApplicationMessenger.h"
#include "filesystem\DrivesManager.h"
#include "guilib\dialogs\GUIDialogSeekBar.h"
#include "guilib\dialogs\GUIDialogVolumeBar.h"
#include "guilib\dialogs\GUIDialogMuteBug.h"
#include "utils\Idle.h"

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
	virtual void DoRender();
	virtual void Cleanup();
	virtual void Stop();

	void StartServices();
	void StopServices();

	virtual bool OnMessage(CGUIMessage& message);
	bool NeedRenderFullScreen();
	void RenderFullScreen();
	void DoRenderFullScreen();
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
	void SeekTime( double dTime = 0.0 );
	float GetPercentage() const;

	bool IsPlayingAudio() const;
	bool IsPlayingVideo() const;

	void StartIdleThread();
	void StopIdleThread();
	void StartTimeServer();
	void StopTimeServer();
	void StartFtpServer();
	void StopFtpServer();

	bool IsCurrentThread() const;

	void ResetScreenSaver();
	bool ResetScreenSaverWindow();
	bool IsInScreenSaver() { return m_bScreenSave; };
	void CheckScreenSaver();

	int GetVolume() const;
	void SetVolume(int iPercent);
	void SetHardwareVolume(long hardwareVolume);
	void Mute(void);

	DWORD m_dwSkinTime;
	IPlayer* m_pPlayer;

	CGUIDialogVolumeBar m_guiDialogVolumeBar;
	CGUIDialogSeekBar m_guiDialogSeekBar;
	CGUIDialogMuteBug m_guiDialogMuteBug;

	CIdleThread& GetIdleThread() { return m_idleThread; };
	CNetwork& getNetwork() { return m_network; };
	CDrivesManager& getDriveManager() { return m_drivesManager; };
	CApplicationMessenger& getApplicationMessenger() { return m_applicationMessenger; };

	bool ExecuteXBMCAction(std::string action);
	void RenderMemoryStatus();

protected:
	bool ProcessGamepad(float frameTime);
	bool OnKey(CKey& key);
	bool OnAction(CAction &action);
	void FatalErrorHandler(bool InitD3D);
	void ActivateScreenSaver();
	void ProcessSlow();

	bool m_bScreenSave;
	CStdString m_screenSaverMode;
	bool m_bInitializing;
	bool m_bPlaybackStarting;
	int m_iPlaySpeed;

	// Timer information
	CStopWatch m_frameTime;
	CStopWatch m_screenSaverTimer;
	CStopWatch m_slowTimer;

	CIdleThread m_idleThread;
	CSplash *m_splash;
	DWORD m_threadID; // Application thread ID. Used in applicationMessanger to know where we are firing a thread with delay from
	CNetwork m_network;
	CNTPClient *m_pNTPClient;
	CFTPServer *m_pFTPServer;
	CDrivesManager m_drivesManager;
	CApplicationMessenger m_applicationMessenger;
};

extern CApplication g_application;

#endif //H_CAPPLICATION
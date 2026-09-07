#ifndef H_CAPPLICATION
#define H_CAPPLICATION

#include "XBApplicationEx.h"
#include "cores\IPlayer.h"
#include "guilib\GUIUserMessages.h"
#include "FileItem.h"
#include "video\Bookmark.h"
#include "guilib\IMsgTargetCallback.h"
#include "utils\Stopwatch.h"
#include "utils\Splash.h"
#include "xbox\Network.h"
#include "services\NTPClient\WinSckNTP.h"
#include "services\FTPServer\FTPServer.h"
#include "ApplicationMessenger.h"
#include "utils\DelayController.h"
#include "filesystem\DrivesManager.h"
#include "guilib\dialogs\GUIDialogSeekBar.h"
#include "guilib\dialogs\GUIDialogVolumeBar.h"
#include "guilib\dialogs\GUIDialogMuteBug.h"
#include "utils\Idle.h"
#include "cores\PlayerCoreFactory.h"
#ifdef HAS_UPNP
#include "network\UPnP.h"
#endif
#ifdef HAS_WEB_SERVER
#include "network\WebServer.h"
#endif

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
	bool LoadUserWindows();

	virtual bool Initialize();
	virtual void Process();
	virtual void FrameMove();
	virtual void Render();
	virtual void DoRender();
	virtual void Cleanup();
	virtual void Stop();

	void StartServices();
	void StopServices();

	CFileItem& CurrentFileItem() { return *m_itemCurrentFile; }
	const CStdString& CurrentFile();
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

	bool PlayFile(const CFileItem& item, bool bRestart = false);
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

	void SaveFileState();
	void UpdateFileState();

	void StartIdleThread();
	void StopIdleThread();
	void StartTimeServer();
	void StopTimeServer();
	void StartFtpServer();
	void StopFtpServer();
#ifdef HAS_WEB_SERVER
	bool StartWebServer();
	void StopWebServer();
#endif
#ifdef HAS_UPNP
	void StartUPnP();
	void StopUPnP();
	void StartUPnPRenderer();
	void StopUPnPRenderer();
#endif

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

	CDelayController m_ctrDpad;

	CIdleThread& GetIdleThread() { return m_idleThread; };
	CNetwork& getNetwork() { return m_network; };
	CDrivesManager& getDriveManager() { return m_drivesManager; };
	CApplicationMessenger& getApplicationMessenger() { return m_applicationMessenger; };

	bool ExecuteXBMCAction(std::string action);
	void RenderMemoryStatus();

	CStdString m_strPlayListFile;

protected:
	bool ProcessGamepad(float frameTime);
	bool ProcessJsonRpcButtons();
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

	CFileItemPtr m_itemCurrentFile;
	int m_nextPlaylistItem;

	CFileItemPtr m_progressTrackingItem;
	bool m_progressTrackingPlayCountUpdate;
	CBookmark m_progressTrackingVideoResumeBookmark;

	// Timer information
	CStopWatch m_frameTime;
	CStopWatch m_screenSaverTimer;
	CStopWatch m_slowTimer;

	CIdleThread m_idleThread;
	CSplash *m_splash;
	DWORD m_threadID; // Application thread ID. Used in applicationMessanger to know where we are firing a thread with delay from
	PLAYERCOREID m_eCurrentPlayer;
	CNetwork m_network;
	CNTPClient *m_pNTPClient;
	CFTPServer *m_pFTPServer;
#ifdef HAS_WEB_SERVER
	CWebServer m_WebServer;
#endif
	CDrivesManager m_drivesManager;
	CApplicationMessenger m_applicationMessenger;
};

extern CApplication g_application;

#endif //H_CAPPLICATION
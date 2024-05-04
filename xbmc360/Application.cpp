#include "Application.h"
#include "guilib\GraphicContext.h"
#include "utils\Log.h"
#include "utils\Util.h"
#include "utils\SingleLock.h"
#include "GUISettings.h"
#include "guilib\GUIWindowManager.h"
#include "cores\VideoRenderers\RenderManager.h"
#include "guilib\GUIFontManager.h"
#include "guilib\GUIInfoManager.h"
#include "cores\DVDPlayer\DVDPlayer.h"
#include "guilib\LocalizeStrings.h"
#include "Settings.h"
#include "filesystem\File.h"
#include "ApplicationMessenger.h"
#include "ButtonTranslator.h"
#include "guilib\AudioContext.h"
#include "guilib\GUIAudioManager.h"
#include "cores\PlayerCoreFactory.h"
#include "AdvancedSettings.h"
#include "utils\URIUtils.h"
#include "guilib\SkinInfo.h"
#include "interfaces\Builtins.h"
#include "guilib\GUIColorManager.h"

// Window includes
#include "guilib\windows\GUIWindowHome.h"
#include "guilib\windows\GUIWindowPrograms.h"
#include "guilib\windows\GUIWindowFullScreen.h"
#include "guilib\windows\GUIWindowVideoFiles.h"
#include "guilib\windows\GUIWindowMusicFiles.h"
#include "guilib\windows\GUIWindowPictures.h"
#include "guilib\windows\GUIWindowSettings.h"
#include "guilib\windows\GUIWindowSettingsCategory.h"
#include "guilib\windows\GUIWindowScreensaver.h"
#include "guilib\windows\GUIWindowSystemInfo.h"

// Dialog includes
#include "guilib\dialogs\GUIDialogButtonMenu.h"
#include "guilib\dialogs\GUIDialogNetworkSetup.h"
#include "guilib\dialogs\GUIDialogMediaSource.h"
#include "guilib\dialogs\GUIDialogContextMenu.h"
#include "guilib\dialogs\GUIDialogYesNo.h"
#include "guilib\dialogs\GUIDialogProgress.h"
#include "guilib\dialogs\GUIDialogKeyboard.h"
#include "guilib\dialogs\GUIDialogNumeric.h"
#include "guilib\dialogs\GUIDialogOK.h"

CStdString g_LoadErrorStr;

CApplication::CApplication() 
{
	m_pPlayer = NULL;
	m_iPlaySpeed = 1;
	m_bPlaybackStarting = false;
	m_dwSkinTime = 0;
	m_bScreenSave = false;
	m_bInitializing = true;
	m_splash = NULL;
}

CApplication::~CApplication()
{
}

bool CApplication::Create()
{
#ifdef _DEBUG
	g_advancedSettings.m_logLevel = LOG_LEVEL_DEBUG;
#else
	g_advancedSettings.m_logLevel = LOG_LEVEL_NORMAL;
#endif
	CLog::SetLogLevel(g_advancedSettings.m_logLevel);
	g_guiSettings.Initialize();  // Initialize default GUI Settings
	g_settings.Initialize(); // Initialize default Settings

	// Check logpath
	CStdString strLogFile, strLogFileOld;
	URIUtils::AddSlashAtEnd(g_settings.m_logFolder);
	strLogFile.Format("%sxbmc.log", g_settings.m_logFolder);
	strLogFileOld.Format("%sxbmc.old.log", g_settings.m_logFolder);

	// Rotate the log (xbmc.log -> xbmc.old.log)
	::DeleteFile(strLogFileOld.c_str());
	::MoveFile(strLogFile.c_str(), strLogFileOld.c_str());

	CLog::Log(LOGNOTICE, "-----------------------------------------------------------------------");
	CLog::Log(LOGNOTICE, "Starting XBox Media Center 360.  Built on %s", __DATE__);
	CLog::Log(LOGNOTICE, "-----------------------------------------------------------------------");

	CLog::Log(LOGNOTICE, "Setup DirectX");

	// Create the Direct3D object
	if(!(m_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
	{
		CLog::Log(LOGFATAL, "Unable to create Direct3D!");
		Sleep(INFINITE); // die
	}

	// Transfer the resolution information to our graphics context
	g_graphicsContext.SetD3DParameters(&m_d3dpp);
	g_graphicsContext.SetVideoResolution(g_guiSettings.m_LookAndFeelResolution, TRUE);

	if(m_pD3D->CreateDevice(0, D3DDEVTYPE_HAL, NULL, D3DCREATE_HARDWARE_VERTEXPROCESSING, &m_d3dpp, &m_pd3dDevice) != S_OK)
	{
		CLog::Log(LOGFATAL, "Unable to create D3D Device!");
		Sleep(INFINITE); // die
	}

	g_graphicsContext.SetD3DDevice(m_pd3dDevice);

	// Set GUI res and force the clear of the screen
	g_graphicsContext.SetVideoResolution(g_guiSettings.m_LookAndFeelResolution, TRUE, true);

	m_splash = new CSplash("D:\\media\\splash.png");
	m_splash->Start();

	int iResolution = g_graphicsContext.GetVideoResolution();
	CLog::Log(LOGINFO, "GUI format %ix%i %s",
		g_settings.m_ResInfo[iResolution].iWidth,
		g_settings.m_ResInfo[iResolution].iHeight,
		g_settings.m_ResInfo[iResolution].strMode);

	// Mount our drives
	CLog::Log(LOGNOTICE, "Mounting the drives..");
	m_drivesManager.Init();

	if(!m_drivesManager.MountAll())
		CLog::Log(LOGERROR, "Failed to mount..");

	// Start XAudio2
	if(!g_audioContext.Initialize())
		CLog::Log(LOGERROR, "Unable to initialize XAudio2!");

	CLog::Log(LOGNOTICE, "Load settings...");	
	if(!g_settings.Load())
		FatalErrorHandler(true);

	// Load the langinfo to have user charset <-> utf-8 conversion
	CStdString strLanguage = g_guiSettings.GetString("locale.language");
	strLanguage[0] = toupper(strLanguage[0]);

	CStdString strLanguagePath = "D:\\language\\";

	CLog::Log(LOGINFO, "load %s language file, from path: %s", strLanguage.c_str(), strLanguagePath.c_str());
	if (!g_localizeStrings.Load(strLanguagePath, strLanguage))
		FatalErrorHandler(true);

	CLog::Log(LOGINFO, "Load keymapping");
	if(!g_buttonTranslator.Load())
		FatalErrorHandler(true);

	return CXBApplicationEX::Create();
}

// This function does not return!
void CApplication::FatalErrorHandler(bool InitD3D)
{
	// XBMC couldn't start for some reason...
	// g_LoadErrorStr should contain the reason
	CLog::Log(LOGWARNING, "Emergency recovery console starting...");

	if(m_splash)
		m_splash->Stop();

	//
	// TODO
	//
}

bool CApplication::Initialize()
{
	// Setup network based on our settings
	// network will start it's init procedure
	m_network.SetupNetwork();

	StartServices();

	g_windowManager.Add(new CGUIWindowHome); // window id = 0

	CLog::Log(LOGNOTICE, "load default skin:[%s]", g_guiSettings.GetString("lookandfeel.skin").c_str());
	LoadSkin(g_guiSettings.GetString("lookandfeel.skin"));

	// Windows
	g_windowManager.Add(new CGUIWindowFullScreen);
	g_windowManager.Add(new CGUIWindowPrograms);
	g_windowManager.Add(new CGUIWindowVideoFiles);
	g_windowManager.Add(new CGUIWindowMusicFiles);
	g_windowManager.Add(new CGUIWindowPictures);	
	g_windowManager.Add(new CGUIWindowSettings);
	g_windowManager.Add(new CGUIWindowSettingsCategory);
	g_windowManager.Add(new CGUIWindowScreensaver);
	g_windowManager.Add(new CGUIWindowSystemInfo);

	// Dialogs
	g_windowManager.Add(new CGUIDialogYesNo);           // window id = 100
	g_windowManager.Add(new CGUIDialogProgress);        // window id = 101
	g_windowManager.Add(new CGUIDialogKeyboard);        // window id = 103
	g_windowManager.Add(new CGUIDialogContextMenu);     // window id = 106
	g_windowManager.Add(new CGUIDialogNumeric);         // window id = 109
	g_windowManager.Add(new CGUIDialogButtonMenu);      // window id = 111
	g_windowManager.Add(&m_guiDialogVolumeBar);         // window id = 104
	g_windowManager.Add(&m_guiDialogSeekBar);           // window id = 115
	g_windowManager.Add(new CGUIDialogNetworkSetup);    // window id = 128
	g_windowManager.Add(new CGUIDialogMediaSource);     // window id = 129
	g_windowManager.Add(new CGUIDialogOK);              // window id = 2002

	g_windowManager.SetCallback(*this);

	g_windowManager.Initialize();

	//  Show mute symbol
	if (g_settings.m_nVolumeLevel == VOLUME_MINIMUM)
		Mute();

	m_slowTimer.StartZero();

	g_windowManager.ActivateWindow(WINDOW_HOME);

	if(m_splash)
		m_splash->Stop();

	SAFE_DELETE(m_splash);

	CLog::Log(LOGNOTICE, "Initialize done");

	m_bInitializing = false;

	// Reset our screensaver (starts timers etc.)
	ResetScreenSaver();

	return true;
}

void CApplication::StartServices()
{
	StartIdleThread();
}

void CApplication::StopServices()
{
	m_network.NetworkMessage(CNetwork::SERVICES_DOWN, 0);

	StopIdleThread();
}

void CApplication::DelayLoadSkin()
{
	m_dwSkinTime = GetTickCount() + 2000;
	return;
}

void CApplication::CancelDelayLoadSkin()
{
	m_dwSkinTime = 0;
}

void CApplication::LoadSkin(const CStdString& strSkin)
{
	m_dwSkinTime = 0;

	CStdString strHomePath;
	CStdString strSkinPath = "D:\\skins\\";
	strSkinPath += strSkin;

	CLog::Log(LOGINFO, "Load skin from:%s", strSkinPath.c_str());

	if(IsPlaying())
	{
		CLog::Log(LOGINFO, "Stop playing...");
		m_pPlayer->CloseFile();
		delete m_pPlayer;
		m_pPlayer = NULL;
	}

	CLog::Log(LOGINFO, "Delete old skin...");
	UnloadSkin();	

	// Load in the skin.xml file if it exists
	g_SkinInfo.Load(strSkinPath);

	g_graphicsContext.SetMediaDir(strSkinPath);

	g_colorManager.Load(g_guiSettings.GetString("lookandfeel.skincolors"));

	CLog::Log(LOGINFO, "Load fonts for skin...");

	CStdString strFontPath = strSkinPath += "\\Fonts.xml";
	g_fontManager.LoadFonts(strFontPath);

	CLog::Log(LOGINFO, "Initialize new skin...");
	m_guiDialogVolumeBar.AllocResources(true);
	m_guiDialogSeekBar.AllocResources(true);
	m_guiDialogMuteBug.AllocResources(true);
	g_windowManager.AddMsgTarget(this);
	g_windowManager.Initialize();
	g_audioManager.Initialize();
	g_audioManager.Load();

	CLog::Log(LOGINFO, "Skin loaded...");
}

void CApplication::ReloadSkin()
{
	CGUIMessage msg(GUI_MSG_LOAD_SKIN, -1, g_windowManager.GetActiveWindow());
	g_windowManager.SendMessage(msg);
	
	// Reload the skin, restoring the previously focused control.  We need this as
	// the window unload will reset all control states.
	CGUIWindow* pWindow = g_windowManager.GetWindow(g_windowManager.GetActiveWindow());
	int iCtrlID = pWindow->GetFocusedControlID();
	g_application.LoadSkin(g_guiSettings.GetString("lookandfeel.skin"));
	pWindow = g_windowManager.GetWindow(g_windowManager.GetActiveWindow());

	if (pWindow && pWindow->HasSaveLastControl())
	{
		CGUIMessage msg3(GUI_MSG_SETFOCUS, g_windowManager.GetActiveWindow(), iCtrlID, 0);
		pWindow->OnMessage(msg3);
	}
}

void CApplication::UnloadSkin()
{
	// These windows are not handled by the windowmanager (why not?) so we should unload them manually
	CGUIMessage msg(GUI_MSG_WINDOW_DEINIT, 0, 0);
	m_guiDialogMuteBug.OnMessage(msg);
	m_guiDialogMuteBug.ResetControlStates();
	m_guiDialogMuteBug.FreeResources(true);

	g_windowManager.DeInitialize();
	g_TextureManager.Cleanup();
	g_fontManager.Clear();
	g_audioManager.DeInitialize();
	g_infoManager.Clear();
}

void CApplication::Process()
{
	// Check if we need to load a new skin
	if(m_dwSkinTime && GetTickCount() >= m_dwSkinTime)
		ReloadSkin();

	// Dispatch the messages generated by python or other threads to the current window
	g_windowManager.DispatchThreadMessages();

	// Process messages, even if a movie is playing
	m_applicationMessenger.ProcessMessages();

	// Process messages which have to be send to the gui
	// (this can only be done after g_windowManager.Render())
	m_applicationMessenger.ProcessWindowMessages();

	// Do any processing that isn't needed on each run
	if(m_slowTimer.GetElapsedMilliseconds() > 500)
	{
		m_slowTimer.Reset();
		ProcessSlow();
	}
}

// We get called every 500ms
void CApplication::ProcessSlow()
{
	// Check our network state every 15 seconds or when net status changes
	m_network.CheckNetwork(30);

	// Check for any needed sntp update
	if(m_pNTPClient && m_pNTPClient->UpdateNeeded())
		m_pNTPClient->SyncTime();

	// Check if we need to activate the screensaver (if enabled)
	if(g_guiSettings.GetString("screensaver.mode") != "None")
		CheckScreenSaver();
}

// Handle the gamepad button presses.  We check for button down,
// then call OnKey() which handles the translation to actions, and sends the
// action to our window manager's OnAction() function, which filters the messages
// to where they're supposed to end up, returning true if the message is successfully
// processed.  If OnKey() returns false, then the key press wasn't processed at all,
// and we can safely process the next key (or next check on the same key in the
// case of the analog sticks which can produce more than 1 key event.)
bool CApplication::ProcessGamepad(float frameTime)
{
	if(m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_A)
	{
		CKey key(KEY_BUTTON_A);		
		if (OnKey(key)) return true;
	}

	if(m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_B)
	{
		CKey key(KEY_BUTTON_B);		
		if (OnKey(key)) return true;
	}

	if(m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_X)
	{
		CKey key(KEY_BUTTON_X);		
		if (OnKey(key)) return true;
	}

	if(m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_Y)
	{
		CKey key(KEY_BUTTON_Y);		
		if (OnKey(key)) return true;
	}

	if(m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_DPAD_UP)
	{
		CKey key(KEY_BUTTON_DPAD_UP);		
		if (OnKey(key)) return true;
	}	
	
	if(m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_DPAD_DOWN)
	{
		CKey key(KEY_BUTTON_DPAD_DOWN);		
		if (OnKey(key)) return true;
	}	

	if(m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_DPAD_LEFT)
	{
		CKey key(KEY_BUTTON_DPAD_LEFT);		
		if (OnKey(key)) return true;
	}	
	
	if(m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
	{
		CKey key(KEY_BUTTON_DPAD_RIGHT);		
		if (OnKey(key)) return true;
	}	

	if(m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_START)
	{
		CKey key(KEY_BUTTON_START);		
		if (OnKey(key)) return true;
	}

	if(m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_BACK)
	{
		CKey key(KEY_BUTTON_BACK);		
		if (OnKey(key)) return true;
	}

	BYTE bLeftTrigger = 0; // TODO //m_DefaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_LEFT_TRIGGER];
	BYTE bRightTrigger = 0; // TODO //m_DefaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER];

	if (m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_LEFT_THUMB)
	{
		CKey key(KEY_BUTTON_LEFT_THUMB_BUTTON, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
		if (OnKey(key)) return true;
	}
	if (m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_RIGHT_THUMB)
	{
		CKey key(KEY_BUTTON_RIGHT_THUMB_BUTTON, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
		if (OnKey(key)) return true;
	}

	if(m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
	{
		CKey key(KEY_BUTTON_LEFT_SHOULDER);		
		if (OnKey(key)) return true;
	}

	if(m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
	{
		CKey key(KEY_BUTTON_RIGHT_SHOULDER);		
		if (OnKey(key)) return true;
	}

	// Map all controller & remote actions to their keys
	if (m_DefaultGamepad.fX1 || m_DefaultGamepad.fY1)
	{
		CKey key(KEY_BUTTON_LEFT_THUMB_STICK, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
		if (OnKey(key)) return true;
	}

	if (m_DefaultGamepad.fX2 || m_DefaultGamepad.fY2)
	{
		CKey key(KEY_BUTTON_RIGHT_THUMB_STICK, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
		if (OnKey(key)) return true;
	}

	// Direction specific keys (for defining different actions for each direction)
	// We need to be able to know when it last had a direction, so that we can
	// post the reset direction code the next time around (to reset scrolling,
	// fastforwarding and other analog actions)

	// For the sticks, once it is pushed in one direction (eg up) it will only
	// detect movement in that direction of movement (eg up or down) - the other
	// direction (eg left and right) will not be registered until the stick has
	// been recentered for at least 2 frames.

	// First the right stick
	static int lastRightStickKey = 0;
	int newRightStickKey = 0;
	
	if (lastRightStickKey == KEY_BUTTON_RIGHT_THUMB_STICK_UP || lastRightStickKey == KEY_BUTTON_RIGHT_THUMB_STICK_DOWN)
	{
		if (m_DefaultGamepad.fY2 > 0)
			newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_UP;
		else if (m_DefaultGamepad.fY2 < 0)
			newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_DOWN;
		else if (m_DefaultGamepad.fX2 != 0)
		{
			newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_UP;
			//m_DefaultGamepad.fY2 = 0.00001f; // small amount of movement
		}
	}
	else if (lastRightStickKey == KEY_BUTTON_RIGHT_THUMB_STICK_LEFT || lastRightStickKey == KEY_BUTTON_RIGHT_THUMB_STICK_RIGHT)
	{
		if (m_DefaultGamepad.fX2 > 0)
			newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_RIGHT;
		else if (m_DefaultGamepad.fX2 < 0)
			newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_LEFT;
		else if (m_DefaultGamepad.fY2 != 0)
		{
			newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_RIGHT;
			//m_DefaultGamepad.fX2 = 0.00001f; // small amount of movement
		}
	}
	else
	{
		if (m_DefaultGamepad.fY2 > 0 && m_DefaultGamepad.fX2*2 < m_DefaultGamepad.fY2 && -m_DefaultGamepad.fX2*2 < m_DefaultGamepad.fY2)
			newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_UP;
		else if (m_DefaultGamepad.fY2 < 0 && m_DefaultGamepad.fX2*2 < -m_DefaultGamepad.fY2 && -m_DefaultGamepad.fX2*2 < -m_DefaultGamepad.fY2)
			newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_DOWN;
		else if (m_DefaultGamepad.fX2 > 0 && m_DefaultGamepad.fY2*2 < m_DefaultGamepad.fX2 && -m_DefaultGamepad.fY2*2 < m_DefaultGamepad.fX2)
			newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_RIGHT;
		else if (m_DefaultGamepad.fX2 < 0 && m_DefaultGamepad.fY2*2 < -m_DefaultGamepad.fX2 && -m_DefaultGamepad.fY2*2 < -m_DefaultGamepad.fX2)
			newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_LEFT;
	}

	if (lastRightStickKey && newRightStickKey != lastRightStickKey)
	{
		// Was held down last time - and we have a new key now
		// post old key reset message...
		CKey key(lastRightStickKey/*, 0, 0, 0, 0, 0, 0*/);
		lastRightStickKey = newRightStickKey;

		if (OnKey(key)) return true;
	}

	lastRightStickKey = newRightStickKey;

	// Post the new key's message
	if (newRightStickKey)
	{
		CKey key(newRightStickKey, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
		if (OnKey(key))	return true;
	}

	// Now the left stick
	static int lastLeftStickKey = 0;
	int newLeftStickKey = 0;
	
	if (lastLeftStickKey == KEY_BUTTON_LEFT_THUMB_STICK_UP || lastLeftStickKey == KEY_BUTTON_LEFT_THUMB_STICK_DOWN)
	{
		if (m_DefaultGamepad.fY1 > 0)
			newLeftStickKey = KEY_BUTTON_LEFT_THUMB_STICK_UP;
		else if (m_DefaultGamepad.fY1 < 0)
			newLeftStickKey = KEY_BUTTON_LEFT_THUMB_STICK_DOWN;
	}
	else if (lastLeftStickKey == KEY_BUTTON_LEFT_THUMB_STICK_LEFT || lastLeftStickKey == KEY_BUTTON_LEFT_THUMB_STICK_RIGHT)
	{
		if (m_DefaultGamepad.fX1 > 0)
			newLeftStickKey = KEY_BUTTON_LEFT_THUMB_STICK_RIGHT;
		else if (m_DefaultGamepad.fX1 < 0)
			newLeftStickKey = KEY_BUTTON_LEFT_THUMB_STICK_LEFT;
	}
	else
	{ 
		// Check for a new control movement
		if (m_DefaultGamepad.fY1 > 0 && m_DefaultGamepad.fX1 < m_DefaultGamepad.fY1 && -m_DefaultGamepad.fX1 < m_DefaultGamepad.fY1)
			newLeftStickKey = KEY_BUTTON_LEFT_THUMB_STICK_UP;
		else if (m_DefaultGamepad.fY1 < 0 && m_DefaultGamepad.fX1 < -m_DefaultGamepad.fY1 && -m_DefaultGamepad.fX1 < -m_DefaultGamepad.fY1)
			newLeftStickKey = KEY_BUTTON_LEFT_THUMB_STICK_DOWN;
		else if (m_DefaultGamepad.fX1 > 0 && m_DefaultGamepad.fY1 < m_DefaultGamepad.fX1 && -m_DefaultGamepad.fY1 < m_DefaultGamepad.fX1)
			newLeftStickKey = KEY_BUTTON_LEFT_THUMB_STICK_RIGHT;
		else if (m_DefaultGamepad.fX1 < 0 && m_DefaultGamepad.fY1 < -m_DefaultGamepad.fX1 && -m_DefaultGamepad.fY1 < -m_DefaultGamepad.fX1)
			newLeftStickKey = KEY_BUTTON_LEFT_THUMB_STICK_LEFT;
	}

	if (lastLeftStickKey && newLeftStickKey != lastLeftStickKey)
	{
		// Was held down last time - and we have a new key now
		// post old key reset message...
		CKey key(lastLeftStickKey/*, 0, 0, 0, 0, 0, 0*/);
		lastLeftStickKey = newLeftStickKey;

		if (OnKey(key))	return true;
	}
	
	lastLeftStickKey = newLeftStickKey;
	
	// Post the new key's message
	if (newLeftStickKey)
	{
		CKey key(newLeftStickKey, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
		if (OnKey(key))	return true;
	}

	return false;
}

// OnKey() translates the key into a CAction which is sent on to our Window Manager.
// The window manager will return true if the event is processed, false otherwise.
// If not already processed, this routine handles global keypresses.  It returns
// true if the key has been processed, false otherwise.
bool CApplication::OnKey(CKey& key) // FIXME : Needs updating to match!
{
	// Get the current active window
	int iWin = g_windowManager.GetActiveWindow();

	// A key has been pressed.
	// Reset the screensaver timer
	// but not for the analog thumbsticks/triggers

//	if(!key.IsAnalogButton()) //TODO
	{
		ResetScreenSaver();
		if (ResetScreenSaverWindow())
			return true;
	}

	// Change this if we have a dialog up
	if (g_windowManager.HasModalDialog())
	{
		iWin = g_windowManager.GetTopMostModalDialogID();
	}

	CAction action = g_buttonTranslator.GetAction(iWin, key);

	bool bResult = false;

	// Play sound before the action unless the button is held, 
	// where we execute after the action as held actions aren't fired every time.
	if(/*action.GetHoldTime()*/false) //TODO
	{
		bResult = OnAction(action);
		if(bResult)
			g_audioManager.PlayActionSound(action);
	}
	else
	{
		g_audioManager.PlayActionSound(action);
		bResult = OnAction(action);
	}

	return bResult;
}

bool CApplication::OnAction(CAction &action)
{
	// Special case for switching between GUI & fullscreen mode.
	if (action.GetID() == ACTION_SHOW_GUI)
	{ 
		// Switch to fullscreen mode if we can
		if (SwitchToFullScreen())
			return true;
	}

	// In normal case
	// just pass the action to the current window and let it handle it
	if (g_windowManager.OnAction(action))
	{
//		m_navigationTimer.StartZero(); // TODO
		return true;
	}

	/* Handle extra global presses */

	// codec info : Shows the current song, video or picture codec information
	if (action.GetID() == ACTION_SHOW_CODEC)
	{
		g_infoManager.ToggleShowCodec();
		return true;
	}

	// Stop : Stops playing current audio song
	if (action.GetID() == ACTION_STOP)
	{
		StopPlaying();
		return true;
	}

	if (IsPlaying())
	{
		// Pause : Pauses current audio song
		if (action.GetID() == ACTION_PAUSE)
		{
			m_pPlayer->Pause();
			return true;
		}
	}

	// Check for global volume control
	if (action.GetAmount() && (action.GetID() == ACTION_VOLUME_UP || action.GetID() == ACTION_VOLUME_DOWN)) // TODO: GetAmount()
	{
		// Increase or decrease the volume
		int volume = g_settings.m_nVolumeLevel + g_settings.m_dynamicRangeCompressionLevel;

		// Calculate speed so that a full press will equal 1 second from min to max
		float speed = float(VOLUME_MAXIMUM - VOLUME_MINIMUM);

		if(action.GetRepeat())
			speed *= action.GetRepeat();
		else
			speed /= 50; // 50 fps
		
		if (g_settings.m_bMute)
		{
			// Only unmute if volume is to be increased, otherwise leave muted
			if (action.GetID() == ACTION_VOLUME_DOWN)
				return true;
      
			if (g_settings.m_iPreMuteVolumeLevel == 0) 
				SetVolume(1); 
			else 
			{
				// In muted, unmute 
				Mute();
			}
			return true;
		}

		if (action.GetID() == ACTION_VOLUME_UP)
			volume += (int)((float)fabs(action.GetAmount()) * action.GetAmount() * speed);
		else
			volume -= (int)((float)fabs(action.GetAmount()) * action.GetAmount() * speed);

		SetHardwareVolume(volume);
		g_audioManager.SetVolume(g_settings.m_nVolumeLevel);

		// Show visual feedback of volume change...
		m_guiDialogVolumeBar.Show();
		m_guiDialogVolumeBar.OnAction(action);
		
		return true;
	}

	return false;
}

void CApplication::FrameMove()
{
	// Currently we calculate the repeat time (ie time from last similar keypress) just global as fps
	float frameTime = m_frameTime.GetElapsedSeconds();
	m_frameTime.StartZero();

	// Never set a frametime less than 2 fps to avoid problems when debuggin and on breaks
	if(frameTime > 0.5) frameTime = 0.5;

	ReadInput(); // Read raw inputs

	// Process input actions
	ProcessGamepad(frameTime);

	g_windowManager.FrameMove();
}

bool CApplication::OnMessage(CGUIMessage& message)
{
	switch (message.GetMessage())
	{
		case GUI_MSG_PLAYBACK_STOPPED:
		case GUI_MSG_PLAYBACK_ENDED:
		{
			if (/*message.GetMessage() == GUI_MSG_PLAYBACK_ENDED*/0) // WIP: Always delete the player on close atm
			{
				// sending true to PlayNext() effectively passes bRestart to PlayFile()
				// which is not generally what we want (except for stacks, which are
				// handled above)
//				g_playlistPlayer.PlayNext();
			}
			else
			{
				delete m_pPlayer;
				m_pPlayer = NULL;
			}

			if(!IsPlaying())
				g_audioManager.Enable(true);

			if (!IsPlayingVideo() && g_windowManager.GetActiveWindow() == WINDOW_FULLSCREEN_VIDEO)
			{
				g_windowManager.PreviousWindow();
			}
			return true;
		}
		break;

		case GUI_MSG_EXECUTE:
			if (message.GetStringParam().length() > 0)
				return ExecuteXBMCAction(message.GetStringParam());
		break;
	}
	return false;
}

void CApplication::Render()
{
	if(!m_pd3dDevice)
		return;

	// Don't do anything that would require graphiccontext to be locked before here in fullscreen.
	// that stuff should go into renderfullscreen instead as that is called from the rendering thread

	// Don't show GUI when playing full screen video
	if(g_graphicsContext.IsFullScreenVideo() /*&& IsPlaying() && !IsPaused()*/)
	{
		Sleep(10);
		ResetScreenSaver();
//		g_infoManager.ResetCache();
		return;
	}

	// Enable/Disable video overlay window
	if(IsPlayingVideo() && g_windowManager.GetActiveWindow() != WINDOW_FULLSCREEN_VIDEO && !m_bScreenSave)
		g_graphicsContext.EnablePreviewWindow(true);
	else
		g_graphicsContext.EnablePreviewWindow(false);

	g_graphicsContext.TLock();
	m_pd3dDevice->BeginScene();  
	g_graphicsContext.TUnlock();

	// Update our FPS
	g_infoManager.UpdateFPS();

	// Draw GUI

	// Render current windows
	g_windowManager.Render();

	// Now render any dialogs
	g_windowManager.RenderDialogs();

    // reset the window scaling and fade status
	g_graphicsContext.SetRenderingResolution(g_graphicsContext.GetVideoResolution(), false);

	g_graphicsContext.TLock();
	m_pd3dDevice->EndScene();
	g_graphicsContext.TUnlock();

	// Present the backbuffer contents to the display
	g_graphicsContext.TLock();
	m_pd3dDevice->Present(NULL, NULL, NULL, NULL);
	g_graphicsContext.TUnlock();
}

bool CApplication::NeedRenderFullScreen()
{
	if(g_windowManager.GetActiveWindow() == WINDOW_FULLSCREEN_VIDEO)
	{
		if(g_windowManager.HasDialogOnScreen()) return true;
 
		CGUIWindowFullScreen *pFSWin = (CGUIWindowFullScreen*)g_windowManager.GetWindow(WINDOW_FULLSCREEN_VIDEO);
		if(!pFSWin)
			 return false;

		return pFSWin->NeedRenderFullScreen();
	}
	return false;
}

void CApplication::RenderFullScreen()
{
	if (g_windowManager.GetActiveWindow() == WINDOW_FULLSCREEN_VIDEO)
	{
//		m_guiVideoOverlay.Close(true); //TODO
//		m_guiMusicOverlay.Close(true); //TODO

		CGUIWindowFullScreen *pFSWin = (CGUIWindowFullScreen *)g_windowManager.GetWindow(WINDOW_FULLSCREEN_VIDEO);
		if(!pFSWin)
			return;
	
		pFSWin->RenderFullScreen();
	}
}

// SwitchToFullScreen() returns true if a switch is made, else returns false
bool CApplication::SwitchToFullScreen()
{
	// See if we're playing a video, and are in GUI mode
	if ( IsPlayingVideo() && g_windowManager.GetActiveWindow() != WINDOW_FULLSCREEN_VIDEO)
	{
		// Then switch to fullscreen mode
		g_windowManager.ActivateWindow(WINDOW_FULLSCREEN_VIDEO);
//		g_TextureManager.Flush();
		return true;
	}

	return false;
}

void CApplication::StopPlaying()
{
	int iWin = g_windowManager.GetActiveWindow();

	if(IsPlaying())
	{
		 if(m_pPlayer)
			m_pPlayer->CloseFile();

		// Turn off visualization window when stopping
		if(iWin == WINDOW_FULLSCREEN_VIDEO)
			g_windowManager.PreviousWindow();
	}
}

void CApplication::OnPlayBackEnded()
{
	if(m_bPlaybackStarting)
		return;

	CLog::Log(LOGDEBUG, "%s - Playback has finished", __FUNCTION__);

	CGUIMessage msg(GUI_MSG_PLAYBACK_ENDED, 0, 0);
	g_windowManager.SendThreadMessage(msg);
}

void CApplication::OnPlayBackStarted()
{
	if(m_bPlaybackStarting)
		return;

	CLog::Log(LOGDEBUG, "%s - Playback has started", __FUNCTION__);

	CGUIMessage msg(GUI_MSG_PLAYBACK_STARTED, 0, 0);
	g_windowManager.SendThreadMessage(msg);
}

void CApplication::OnQueueNextItem()
{
}

void CApplication::OnPlayBackStopped()
{
	if(m_bPlaybackStarting)
		return;

	CLog::Log(LOGDEBUG, "%s - Playback was stopped", __FUNCTION__);

	CGUIMessage msg( GUI_MSG_PLAYBACK_STOPPED, 0, 0 );
	g_windowManager.SendThreadMessage(msg);
}

bool CApplication::PlayFile(const CFileItem& item)
{
	// Tell system we are starting a file
	m_bPlaybackStarting = true;
	CPlayerOptions options;

	if(m_pPlayer)
	{
		// We should restart the player
		delete m_pPlayer;
		m_pPlayer = NULL;
	}

	if(!m_pPlayer)
	{
		// We only have DVDPlayer atm..
		CPlayerCoreFactory factory;
		m_pPlayer = factory.CreatePlayer("dvdplayer", *this);
	}

	bool bResult;
	if(m_pPlayer)
	{
		// Don't hold graphicscontext here since player
		// may wait on another thread, that requires gfx
//		CSingleExit ex(g_graphicsContext);
		options.fullscreen = true;
		bResult = m_pPlayer->OpenFile(item, options);
	}
	else
	{
		CLog::Log(LOGERROR, "Error creating player for item %s (File doesn't exist?)", item.GetPath());
		bResult = false;
	}

	if(bResult)
	{
		if(m_iPlaySpeed != 1)
		{
			int iSpeed = m_iPlaySpeed;
			m_iPlaySpeed = 1;
			SetPlaySpeed(iSpeed);
		}

		if(IsPlayingAudio()) // TODO
		{
//			if (g_windowManager.GetActiveWindow() == WINDOW_FULLSCREEN_VIDEO)
//				g_windowManager.ActivateWindow(WINDOW_VISUALISATION);
		}

		if(IsPlayingVideo())
		{
//			if (g_windowManager.GetActiveWindow() == WINDOW_VISUALISATION)  // TODO
//				g_windowManager.ActivateWindow(WINDOW_FULLSCREEN_VIDEO);

			// if player didn't manage to switch to fullscreen by itself do it here
			if(g_renderManager.IsStarted()
				&& g_windowManager.GetActiveWindow() != WINDOW_FULLSCREEN_VIDEO)
			{
				SwitchToFullScreen();
			}
		}

		g_audioManager.Enable(false);
	}

	m_bPlaybackStarting = false;

	if(bResult)
	{
		// We must have started, otherwise player might send this later
		if(IsPlaying())
			OnPlayBackStarted();
		else
			OnPlayBackEnded();
	}

	return bResult;
}

bool CApplication::IsPlaying() const
{
	if(!m_pPlayer)
		return false;

	if(!m_pPlayer->IsPlaying())
		return false;

	return true;
}

bool CApplication::IsPaused() const
{
	if(!m_pPlayer)
		return false;

	if(!m_pPlayer->IsPlaying())
		return false;
	
	return m_pPlayer->IsPaused();
}

bool CApplication::IsPlayingAudio() const
{
	if(!m_pPlayer)
		return false;

	if(!m_pPlayer->IsPlaying())
		return false;
	
	if(m_pPlayer->HasVideo())
		return false;
	
	if(m_pPlayer->HasAudio())
		return true;
	
	return false;
}

bool CApplication::IsPlayingVideo() const
{
	if(!m_pPlayer)
		return false;

	if(!m_pPlayer->IsPlaying())
		return false;

	if(m_pPlayer->HasVideo())
		return true;

	return false;
}

void CApplication::SetPlaySpeed(int iSpeed)
{
	if(!IsPlayingAudio() && !IsPlayingVideo())
		return;
 
	if(m_iPlaySpeed == iSpeed)
		return;
 
	m_iPlaySpeed = iSpeed;
}

int CApplication::GetPlaySpeed() const
{
	return m_iPlaySpeed;
}

// Returns the current time in seconds of the currently playing media.
// Fractional portions of a second are possible. This returns a double to
// be consistent with GetTotalTime() and SeekTime().
double CApplication::GetTime() const
{
	double dTime = 0.0;

	if(IsPlaying() && m_pPlayer)
		dTime = static_cast<double>(m_pPlayer->GetTime() * 0.001f);
	
	return dTime;
}

// Returns the total time in seconds of the current media. Fractional
// portions of a second are possible - but not necessarily supported by the
// player class.  This returns a double to be consistent with GetTime() and
// SeekTime().
double CApplication::GetTotalTime() const
{
	double dTime = 0.0;

	if(IsPlaying() && m_pPlayer)
		dTime = m_pPlayer->GetTotalTime();
	
	return dTime;
}

float CApplication::GetPercentage() const
{
	if(IsPlaying() && m_pPlayer)
		return m_pPlayer->GetPercentage();

	return 0.0f;
}

void CApplication::StartIdleThread()
{
	m_idleThread.Create(false, 0x100);
}

void CApplication::StopIdleThread()
{
	m_idleThread.StopThread();
}

void CApplication::StartTimeServer()
{
	if(/*g_guiSettings.GetBool("locale.timeserver") &&*/ m_network.IsAvailable())
	{
		if(!m_pNTPClient)
		{
			CLog::Log(LOGNOTICE, "Start timeserver client");

			m_pNTPClient = new CNTPClient();
			m_pNTPClient->SyncTime();
		}
	}
}

void CApplication::StopTimeServer()
{
	if(m_pNTPClient)
	{
		CLog::Log(LOGNOTICE, "Stop time server client");
		SAFE_DELETE(m_pNTPClient);
	}
}

void CApplication::StartFtpServer()
{
	if(/*g_guiSettings.GetBool("services.ftpserver") &&*/ m_network.IsAvailable())
	{
		CLog::Log(LOGNOTICE, "FTP Server: Starting...");
		if(!m_pFTPServer)
		{
			m_pFTPServer = new CFTPServer();
			m_pFTPServer->Start();
		}
	}
}

void CApplication::StopFtpServer()
{
	if(m_pFTPServer)
	{
		CLog::Log(LOGINFO, "FTP Server: Stopping...");

		std::vector<CFTPServerConn*> vecConnections;
		m_pFTPServer->GetAllConnections(vecConnections);

		for(int i = 0; i < (int)vecConnections.size(); i++)
		{
			CFTPServerConn* pConnection = vecConnections[i];

			if(pConnection)
			{
				CLog::Log(LOGNOTICE, "%s - Closing FTP connection %i", pConnection->GetID());
				m_pFTPServer->CloseConnection(pConnection->GetID());
			}
		}

		m_pFTPServer->StopThread();
		delete m_pFTPServer;
		m_pFTPServer = NULL;

		CLog::Log(LOGINFO, "FTP Server: Stopped");
	}
}

void CApplication::ResetScreenSaver()
{
	// Reset our timers
//	m_shutdownTimer.StartZero(); //TODO

	// Screen saver timer is reset only if we're not already in screensaver mode
	if(!m_bScreenSave)
		m_screenSaverTimer.StartZero();
}

bool CApplication::ResetScreenSaverWindow()
{
	// If Screen saver is active
	if(m_bScreenSave)
	{
		// Disable screensaver
		m_bScreenSave = false;
		m_screenSaverTimer.StartZero();

		if(m_screenSaverMode != "None")
		{
			// We're in screensaver window
			if (g_windowManager.GetActiveWindow() == WINDOW_SCREENSAVER)
				g_windowManager.PreviousWindow(); // Show the previous window
		}

		return true;
	}
	else
		return false;
}

void CApplication::CheckScreenSaver()
{
	// If the screen saver window is active, then clearly we are already active
	if(g_windowManager.IsWindowActive(WINDOW_SCREENSAVER))
	{
		m_bScreenSave = true;
		return;
	}

	bool resetTimer = false;
	if(IsPlayingVideo() && !m_pPlayer->IsPaused()) // Are we playing video and it is not paused?
		resetTimer = true;

//	if (IsPlayingAudio() && g_windowManager.GetActiveWindow() == WINDOW_VISUALISATION) // Are we playing some music in fullscreen vis?  // TODO
//		resetTimer = true;

	if(resetTimer)
	{
		m_screenSaverTimer.StartZero();
		return;
	}

	if(m_bScreenSave) // Already running the screensaver
		return;

	if(m_screenSaverTimer.GetElapsedSeconds() > g_guiSettings.GetInt("screensaver.time") * 60)
		ActivateScreenSaver();
}

void CApplication::ActivateScreenSaver()
{
	m_bScreenSave = true;

	// Get Screensaver Mode
	m_screenSaverMode = g_guiSettings.GetString("screensaver.mode");

	if(m_screenSaverMode != "None")
	{
		g_windowManager.ActivateWindow(WINDOW_SCREENSAVER);
		return;
	}
}

int CApplication::GetVolume() const
{
	// Converts the hardware volume (in mB) to a percentage
	return int(((float)(g_settings.m_nVolumeLevel + g_settings.m_dynamicRangeCompressionLevel - VOLUME_MINIMUM)) / (VOLUME_MAXIMUM - VOLUME_MINIMUM)*100.0f + 0.5f);
}

void CApplication::SetVolume(int iPercent) // TODO
{
	// Convert the percentage to a mB (milliBell) value (*100 for dB)
	long hardwareVolume = (long)((float)iPercent * 0.01f * (VOLUME_MAXIMUM - VOLUME_MINIMUM) + VOLUME_MINIMUM);

	SetHardwareVolume(hardwareVolume);
	g_audioManager.SetVolume(g_settings.m_nVolumeLevel);
}

void CApplication::SetHardwareVolume(long hardwareVolume)
{
	if (hardwareVolume >= VOLUME_MAXIMUM) // + VOLUME_DRC_MAXIMUM
		hardwareVolume = VOLUME_MAXIMUM;// + VOLUME_DRC_MAXIMUM;
	if (hardwareVolume <= VOLUME_MINIMUM)
		hardwareVolume = VOLUME_MINIMUM;

	// Update our settings
	if (hardwareVolume > VOLUME_MAXIMUM)
	{
		g_settings.m_dynamicRangeCompressionLevel = hardwareVolume - VOLUME_MAXIMUM;
		g_settings.m_nVolumeLevel = VOLUME_MAXIMUM;
	}
	else
	{
		g_settings.m_dynamicRangeCompressionLevel = 0;
		g_settings.m_nVolumeLevel = hardwareVolume;
	}

	// Update mute state
	if(!g_settings.m_bMute && hardwareVolume <= VOLUME_MINIMUM) //TODO: WIP Need mute dialog!!
	{
		g_settings.m_bMute = true;

		if (!m_guiDialogMuteBug.IsDialogRunning())
			m_guiDialogMuteBug.Show();
	}
	else if(g_settings.m_bMute && hardwareVolume > VOLUME_MINIMUM)
	{
		g_settings.m_bMute = false;

		if (m_guiDialogMuteBug.IsDialogRunning())
			m_guiDialogMuteBug.Close();
	}

	// and tell our player to update the volume
	if(m_pPlayer)
		m_pPlayer->SetVolume(g_settings.m_nVolumeLevel);
}

void CApplication::Mute(void)
{
	if (g_settings.m_bMute)
	{
		// Muted - unmute.
		// In case our premutevolume is 0, return to 100% volume
		if(g_settings.m_iPreMuteVolumeLevel == 0)
		{
			SetVolume(100);
		}
		else
		{
			SetVolume(g_settings.m_iPreMuteVolumeLevel);
			g_settings.m_iPreMuteVolumeLevel = 0;
		}
		m_guiDialogVolumeBar.Show();
	}
	else
	{
		// mute
		g_settings.m_iPreMuteVolumeLevel = GetVolume();
		SetVolume(0);
	}
}

bool CApplication::ExecuteXBMCAction(std::string actionStr)
{
	// See if it is a user set string
	CLog::Log(LOGDEBUG,"%s : Translating %s", __FUNCTION__, actionStr.c_str());
	CGUIInfoLabel info(actionStr, "");
	actionStr = info.GetLabel(0);
	CLog::Log(LOGDEBUG,"%s : To %s", __FUNCTION__, actionStr.c_str());

	// User has asked for something to be executed
	if (CBuiltins::HasCommand(actionStr))
			CBuiltins::Execute(actionStr);
	else
	{
		// Try translating the action from our ButtonTranslator
		int actionID;

		if (CButtonTranslator::TranslateActionString(actionStr.c_str(), actionID))
		{
//			OnAction(CAction(actionID));// TODO 
			return true;
		}

		CFileItem item(actionStr, false);

/*		if (item.IsPythonScript()) //TODO
		{
			// A python script
			unsigned int argc = 1;
			char ** argv = new char*[argc];
			argv[0] = (char*)item.GetPath().c_str();
			g_pythonParser.evalFile(argv[0], argc, (const char**)argv);
			delete [] argv;
		}
		else if (item.IsXEX()) //TODO
		{
			// an XEX
			int iRegion;
			if (g_guiSettings.GetBool("myprograms.gameautoregion"))
			{
				CXBE xbe;
				iRegion = xbe.ExtractGameRegion(item.GetPath());

				if (iRegion < 1 || iRegion > 7)
					iRegion = 0;
				
				iRegion = xbe.FilterRegion(iRegion);
			}
			else
				iRegion = 0;
			
			CUtil::RunXEX(item.GetPath().c_str(),NULL,F_VIDEO(iRegion));
		}
		else if (item.IsAudio() || item.IsVideo())
		{
			// An audio or video file
			PlayFile(item);
		}
		else*/
			return false;
	}
	return true;
}

void CApplication::Cleanup()
{
	try
	{
		// Windows
		g_windowManager.Delete(WINDOW_HOME);
		g_windowManager.Delete(WINDOW_FULLSCREEN_VIDEO);
		g_windowManager.Delete(WINDOW_PROGRAMS);
		g_windowManager.Delete(WINDOW_VIDEOS);
		g_windowManager.Delete(WINDOW_MUSIC);
		g_windowManager.Delete(WINDOW_PICTURES);
		g_windowManager.Delete(WINDOW_SETTINGS);
		g_windowManager.Delete(WINDOW_SETTINGS_MYPICTURES); // All the settings categories
		g_windowManager.Delete(WINDOW_SCREENSAVER);
		g_windowManager.Delete(WINDOW_SYSTEM_INFORMATION);

		// Dialogs
		g_windowManager.Delete(WINDOW_DIALOG_YES_NO);
		g_windowManager.Delete(WINDOW_DIALOG_PROGRESS);
		g_windowManager.Delete(WINDOW_DIALOG_CONTEXT_MENU);
		g_windowManager.Delete(WINDOW_DIALOG_BUTTON_MENU);
		g_windowManager.Delete(WINDOW_DIALOG_MEDIA_SOURCE);
		g_windowManager.Delete(WINDOW_DIALOG_NETWORK_SETUP);
		g_windowManager.Delete(WINDOW_DIALOG_KEYBOARD);
		g_windowManager.Delete(WINDOW_DIALOG_NUMERIC);
		g_windowManager.Delete(WINDOW_DIALOG_OK);

		g_windowManager.Remove(WINDOW_DIALOG_SEEK_BAR);
		g_windowManager.Remove(WINDOW_DIALOG_VOLUME_BAR);

		// Reset our d3d params before we destroy
		g_graphicsContext.SetD3DDevice(NULL);
		g_graphicsContext.SetD3DParameters(NULL);

		g_infoManager.Clear();
		g_localizeStrings.Clear();
		g_settings.Clear();
		g_guiSettings.Clear();
		g_advancedSettings.Clear();
		g_buttonTranslator.Clear();
	}
	catch(...)
	{
		CLog::Log(LOGERROR, "Exception in CApplication::Cleanup()");
	}
}

void CApplication::Stop()
{
	try
	{
		CLog::Log(LOGNOTICE, "Storing total System Uptime");
		g_settings.m_iSystemTimeTotalUp = g_settings.m_iSystemTimeTotalUp + (int)(CTimeUtils::GetFrameTime() / 60000);
		
		// Update the settings information (volume, uptime etc. need saving)
		if(XFILE::CFile::Exists("D:\\settings.xml"))
		{
			CLog::Log(LOGNOTICE, "Saving settings");
			g_settings.Save();
		}
		else
			CLog::Log(LOGNOTICE, "Not saving settings (settings.xml is not present)");

		m_bStop = true;

		CLog::Log(LOGNOTICE, "Stop all services");
		StopServices();

		if(m_pPlayer)
		{
			CLog::Log(LOGNOTICE, "Stop player");
			delete m_pPlayer;
			m_pPlayer = NULL;
		}

		m_applicationMessenger.Cleanup();

		CLog::Log(LOGNOTICE, "Unload skin");
		UnloadSkin();

		m_network.Deinitialize();

		// Shutdown XAudio2
		g_audioContext.DeInitialize();

		// Unmount our drives
//		m_drivesManager.Unmount(); //FIXME - Causes a crash atm

		CLog::Log(LOGNOTICE, "Stopped");
	}
	catch(...)
	{
		CLog::Log(LOGERROR, "Exception in CApplication::Stop()");
	}

	Destroy();
}
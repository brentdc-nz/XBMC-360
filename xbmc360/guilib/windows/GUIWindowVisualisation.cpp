#include "GUIWindowVisualisation.h"
#include "guilib\GUIVisualisationControl.h"
#include "Application.h"
#include "guilib\GUIInfoManager.h"
#include "ButtonTranslator.h"
#include "guilib\GUIWindowManager.h"
#include "guilib\GUIUserMessages.h"
#include "Settings.h"
#include "AdvancedSettings.h"
#include "visualizations\Visualisation.h"

#define TRANSISTION_COUNT   50  // 1 Second
#define TRANSISTION_LENGTH 200  // 4 Seconds
#define START_FADE_LENGTH  100  // 2 Seconds on startup

#define CONTROL_VIS          2

CGUIWindowVisualisation::CGUIWindowVisualisation(void)
	: CGUIWindow(WINDOW_VISUALISATION, "MusicVisualisation.xml")
{
	m_dwInitTimer = 0;
	m_dwLockedTimer = 0;
	m_bShowPreset = false;
}

CGUIWindowVisualisation::~CGUIWindowVisualisation(void)
{
}

bool CGUIWindowVisualisation::OnAction(const CAction &action)
{
	switch (action.GetID())
	{
		case ACTION_SHOW_INFO:
		{
			if (!m_dwInitTimer || g_settings.m_bMyMusicSongThumbInVis)
				g_settings.m_bMyMusicSongThumbInVis = !g_settings.m_bMyMusicSongThumbInVis;

			g_infoManager.SetShowInfo(g_settings.m_bMyMusicSongThumbInVis);

			return true;
		}
		break;

		case ACTION_SHOW_GUI:
		// Save the settings
		g_settings.Save();
		g_windowManager.PreviousWindow();
		return true;
		break;

		case ACTION_VIS_PRESET_LOCK:
		{ // Show the locked icon + fall through so that the vis handles the locking
			CGUIMessage msg(GUI_MSG_GET_VISUALISATION, 0, 0);
			g_windowManager.SendMessage(msg);
			if (msg.GetPointer())
			{
				CVisualisation *pVis = (CVisualisation *)msg.GetPointer();
				char **pPresets = NULL;
				int currpreset = 0, numpresets = 0;
				bool locked;

				pVis->GetPresets(&pPresets, &currpreset, &numpresets, &locked);
				if (numpresets == 1 || !pPresets)
					return true;
			}
			if (!m_bShowPreset)
			{
				m_dwLockedTimer = START_FADE_LENGTH;
				g_infoManager.SetShowCodec(true);
			}
		}
		break;
		case ACTION_VIS_PRESET_SHOW:
		{
			if (!m_dwLockedTimer || m_bShowPreset)
				m_bShowPreset = !m_bShowPreset;
			g_infoManager.SetShowCodec(m_bShowPreset);
			return true;
		}
		break;
	}
	
	// Default action is to send to the visualisation first
	CGUIVisualisationControl *pVisControl = (CGUIVisualisationControl *)GetControl(CONTROL_VIS);
	if (pVisControl && pVisControl->OnAction(action))
		return true;
		
	return CGUIWindow::OnAction(action);
}

bool CGUIWindowVisualisation::OnMessage(CGUIMessage& message)
{
	switch ( message.GetMessage() )
	{
		case GUI_MSG_PLAYBACK_STARTED:
		{
			CGUIVisualisationControl *pVisControl = (CGUIVisualisationControl *)GetControl(CONTROL_VIS);
			if (pVisControl)
				return pVisControl->OnMessage(message);
		}
		break;
		case GUI_MSG_GET_VISUALISATION:
		{
			CGUIVisualisationControl *pVisControl = (CGUIVisualisationControl *)GetControl(CONTROL_VIS);
			if (pVisControl)
				message.SetPointer(pVisControl->GetVisualisation());
			return true;
		}
		break;
		case GUI_MSG_VISUALISATION_ACTION:
		{
			CGUIVisualisationControl *pVisControl = (CGUIVisualisationControl *)GetControl(CONTROL_VIS);
			if (pVisControl)
				return pVisControl->OnMessage(message);
		}
		break;
		case GUI_MSG_WINDOW_DEINIT:
		{
			if (IsActive()) // Save any changed settings from the OSD
				g_settings.Save();
			// Check and close any OSD windows
/* //TODO
			CGUIDialog *pOSD = (CGUIDialog *)g_windowManager.GetWindow(WINDOW_DIALOG_MUSIC_OSD);
			if (pOSD && pOSD->IsDialogRunning()) pOSD->Close(true);
			CGUIDialog *pList = (CGUIDialog *)g_windowManager.GetWindow(WINDOW_DIALOG_VIS_PRESET_LIST);
			if (pList && pList->IsDialogRunning()) pList->Close(true);
*/		}
		break;
		case GUI_MSG_WINDOW_INIT:
		{
			// Check whether we've come back here from a window during which time we've actually
			// stopped playing music
			if (message.GetParam1() == WINDOW_INVALID && !g_application.IsPlayingAudio())
			{
				// Why are we here if nothing is playing???
				g_windowManager.PreviousWindow();
				return true;
			}

			// Hide or show the preset button(s)
			g_infoManager.SetShowCodec(m_bShowPreset);
			g_infoManager.SetShowInfo(true); // Always show the info initially.
			
			CGUIWindow::OnMessage(message);
			if (g_infoManager.GetCurrentSongTag())
				m_tag = *g_infoManager.GetCurrentSongTag();

			if (g_settings.m_bMyMusicSongThumbInVis)
			{
				// Always on
				m_dwInitTimer = 0;
			}
			else
			{
				// Start display init timer (fade out after configured duration)
				m_dwInitTimer = g_advancedSettings.m_songInfoDuration * 50;
			}
			return true;
		}
	}
	return CGUIWindow::OnMessage(message);
}

void CGUIWindowVisualisation::FrameMove()
{
	g_application.ResetScreenSaver();
	// Check for a tag change
	const MUSIC_INFO::CMusicInfoTag* tag = g_infoManager.GetCurrentSongTag();
	if (tag && *tag != m_tag)
	{
		// Need to fade in then out again
		m_tag = *tag;
		// Fade in
		m_dwInitTimer = g_advancedSettings.m_songInfoDuration * 50;
		g_infoManager.SetShowInfo(true);
	}
	if (m_dwInitTimer)
	{
		m_dwInitTimer--;
		
		if (!m_dwInitTimer && !g_settings.m_bMyMusicSongThumbInVis)
		{
			// Reached end of fade in, fade out again
			g_infoManager.SetShowInfo(false);
		}
	}
	
	// Show or hide the locked texture
	if (m_dwLockedTimer)
	{
		m_dwLockedTimer--;
		
		if (!m_dwLockedTimer && !m_bShowPreset)
			g_infoManager.SetShowCodec(false);
	}
	CGUIWindow::FrameMove();
}

void CGUIWindowVisualisation::AllocResources(bool forceLoad)
{
	CGUIWindow::AllocResources(forceLoad);
	// TODO: Enable dialog AllocResources once MusicOSD.xml and VisualisationPresetList.xml
	// exist in the skin. Loading nonexistent XMLs sets the dialog ID to WINDOW_INVALID
	// via LoadXML() failure, corrupting window state.
	//CGUIWindow *pWindow;
	//pWindow = g_windowManager.GetWindow(WINDOW_DIALOG_MUSIC_OSD);
	//if (pWindow) pWindow->AllocResources(true);
	//pWindow = g_windowManager.GetWindow(WINDOW_DIALOG_VIS_PRESET_LIST);
	//if (pWindow) pWindow->AllocResources(true);
}

void CGUIWindowVisualisation::FreeResources(bool forceUnload)
{
	// Save changed settings from music OSD
	g_settings.Save();
	// TODO: Enable dialog FreeResources once MusicOSD.xml and VisualisationPresetList.xml
	// exist in the skin.
	//CGUIWindow *pWindow;
	//pWindow = g_windowManager.GetWindow(WINDOW_DIALOG_MUSIC_OSD);
	//if (pWindow) pWindow->FreeResources(true);
	//pWindow = g_windowManager.GetWindow(WINDOW_DIALOG_VIS_PRESET_LIST);
	//if (pWindow) pWindow->FreeResources(true);
	CGUIWindow::FreeResources(forceUnload);
}


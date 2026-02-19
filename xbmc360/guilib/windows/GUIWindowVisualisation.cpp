#include "GUIWindowVisualisation.h"
#include "guilib\GUIVisualisationControl.h"
#include "Application.h"
#include "guilib\GUIInfoManager.h"
#include "ButtonTranslator.h"
#include "guilib\GUIWindowManager.h"
#include "guilib\GUIUserMessages.h"
#include "Settings.h"
#include "AdvancedSettings.h"

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
			//if (!m_dwInitTimer || g_settings.m_bMyMusicSongThumbInVis) // TODO: m_bMyMusicSongThumbInVis not yet implemented
			//	g_settings.m_bMyMusicSongThumbInVis = !g_settings.m_bMyMusicSongThumbInVis;
			//g_infoManager.SetShowInfo(g_settings.m_bMyMusicSongThumbInVis);
			g_infoManager.SetShowInfo(!g_infoManager.GetBool(PLAYER_SHOWINFO));
			m_dwInitTimer = 0; // Cancel any pending auto-hide
			return true;
		}
		break;

		case ACTION_SHOW_GUI:
		// Save the settings
		g_settings.Save();
		g_windowManager.PreviousWindow();
		return true;
		break;
	}
	return CGUIWindow::OnAction(action);
}

bool CGUIWindowVisualisation::OnMessage(CGUIMessage& message)
{
	switch ( message.GetMessage() )
	{
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

			if (/*g_settings.m_bMyMusicSongThumbInVis*/0) // TODO BRENT
			{ 
				// Always on
				m_dwInitTimer = 0;
			}
			else
			{
				// Start display init timer (fade out after 3 secs...)
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
//	const CMusicInfoTag* tag = g_infoManager.GetCurrentSongTag(); // TODO BRENT
/*	
	if (tag && *tag != m_tag)
	{
		// Need to fade in then out again
		m_tag = *tag;

		// Fade in
		m_dwInitTimer = g_advancedSettings.m_songInfoDuration * 50;
		g_infoManager.SetShowInfo(true);
	}
*/	
	if (m_dwInitTimer)
	{
		m_dwInitTimer--;
		
		if (!m_dwInitTimer/* && !g_settings.m_bMyMusicSongThumbInVis*/) // TODO BRENT
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

void CGUIWindowVisualisation::AllocResources(bool forceLoad) // TODO
{
	CGUIWindow::AllocResources(forceLoad);
}

void CGUIWindowVisualisation::FreeResources(bool forceUnload) // TODO
{
	CGUIWindow::FreeResources(forceUnload);
}


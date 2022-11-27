#include "GUIWindowScreensaver.h"
#include "guilib\GUIWindowManager.h"
#include "Application.h"

// Screensaver
#include "guilib\screensavers\ScreensaverPlasma.h"

CGUIWindowScreensaver::CGUIWindowScreensaver(void) : CGUIWindow(WINDOW_SCREENSAVER, "")
{
	m_loadOnDemand = false;
	m_pScreensaver = NULL;
}

CGUIWindowScreensaver::~CGUIWindowScreensaver(void)
{
}

void CGUIWindowScreensaver::Render()
{
	if(m_pScreensaver)
		m_pScreensaver->Render();

	CGUIWindow::Render();
}

bool CGUIWindowScreensaver::OnAction(const CAction &action)
{
	//Return if somthing is pressed
	g_application.ResetScreenSaverWindow(); //TODO Move call back to CApplication::OnKey()

	return CGUIWindow::OnAction(action);
}

bool CGUIWindowScreensaver::OnMessage(CGUIMessage& message)
{
	switch ( message.GetMessage() )
	{
		case GUI_MSG_WINDOW_INIT:
		{
			m_pScreensaver = new CScreensaverPlasma; // TODO: Create a factory class - Only have Plasma screensaver atm tho
			m_pScreensaver->Initialize();

			return true;
		}

		case GUI_MSG_WINDOW_DEINIT:
		{
			if(m_pScreensaver)
			{
				m_pScreensaver->Close();
				delete m_pScreensaver;
				m_pScreensaver = NULL;
			}
			return true;
		}
		break;
	}
	return CGUIWindow::OnMessage(message);
}

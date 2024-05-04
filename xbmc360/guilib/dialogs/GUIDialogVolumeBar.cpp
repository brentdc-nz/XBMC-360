#include "GUIDialogVolumeBar.h"
#include "guilib\GUISliderControl.h"
#include "utils\TimeUtils.h"

#define VOLUME_BAR_DISPLAY_TIME 1000L

CGUIDialogVolumeBar::CGUIDialogVolumeBar(void)
	: CGUIDialog(WINDOW_DIALOG_VOLUME_BAR, "DialogVolumeBar.xml")
{
	m_loadOnDemand = false;
}

CGUIDialogVolumeBar::~CGUIDialogVolumeBar(void)
{
}

bool CGUIDialogVolumeBar::OnAction(const CAction &action)
{
	if(action.GetID() == ACTION_VOLUME_UP || action.GetID() == ACTION_VOLUME_DOWN)
	{
		// Reset the timer, as we've changed the volume level
		ResetTimer();
		return true;
	}

	return CGUIDialog::OnAction(action);
}

bool CGUIDialogVolumeBar::OnMessage(CGUIMessage& message)
{
	switch(message.GetMessage())
	{
		case GUI_MSG_WINDOW_INIT:
		{
			// Resources are allocated in g_application
			CGUIDialog::OnMessage(message);
			ResetTimer();
			return true;
		}
		break;

		case GUI_MSG_WINDOW_DEINIT:
		{
			// Don't deinit, g_application handles it
			return CGUIDialog::OnMessage(message);
		}
		break;
	}

	return false; // Don't process anything other than what we need!
}

void CGUIDialogVolumeBar::ResetTimer()
{
	m_timer = CTimeUtils::GetFrameTime();
}

void CGUIDialogVolumeBar::Render()
{
	// and render the controls
	CGUIDialog::Render();
	
	// Now check if we should exit
	if (CTimeUtils::GetFrameTime() - m_timer > VOLUME_BAR_DISPLAY_TIME)
		Close();
}
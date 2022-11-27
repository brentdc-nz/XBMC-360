#include "GUIDialogSeekBar.h"
#include "Application.h"

#define SEEK_BAR_DISPLAY_TIME 2000L

CGUIDialogSeekBar::CGUIDialogSeekBar(void)
	: CGUIDialog(WINDOW_DIALOG_SEEK_BAR, "DialogSeekBar.xml")
{
}

CGUIDialogSeekBar::~CGUIDialogSeekBar(void)
{
}

void CGUIDialogSeekBar::ResetTimer()
{
	m_timer = CTimeUtils::GetFrameTime();
}

void CGUIDialogSeekBar::Render()
{
	// Check if we should seek or exit
	if (!g_application.m_pPlayer || (GetTickCount() - m_timer > SEEK_BAR_DISPLAY_TIME))
	{
		Close(true);
		return;
	}

	CGUIDialog::Render();
}
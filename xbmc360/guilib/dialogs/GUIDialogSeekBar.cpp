#include "GUIDialogSeekBar.h"
#include "..\..\Application.h"

#define SEEK_BAR_DISPLAY_TIME 2000L

CGUIDialogSeekBar::CGUIDialogSeekBar(void) 
	: CGUIDialog(WINDOW_DIALOG_SEEK_BAR, "DialogSeekbar.xml")
{
}

CGUIDialogSeekBar::~CGUIDialogSeekBar(void)
{
}

void CGUIDialogSeekBar::ResetTimer()
{
	m_dwTimer = GetTickCount();
}

void CGUIDialogSeekBar::Render()
{
	// Check if we should seek or exit
	if (!g_application.m_pPlayer || (GetTickCount() - m_dwTimer > SEEK_BAR_DISPLAY_TIME))
	{
		Close(true);
		return;
	}

	CGUIDialog::Render();
}
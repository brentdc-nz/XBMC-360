#include "GUIWindowHome.h"

#include "..\GraphicContext.h"

#include "..\GUIInfoManager.h"

#include "..\..\utils\StdString.h"

#if 1
#define MB	(1024*1024)
#define AddStr(a,b) (pstrOut += wsprintf( pstrOut, a, b ))
#endif

CGUIWindowHome::CGUIWindowHome(void) : CGUIWindow(WINDOW_HOME, "Home.xml")
{
	m_loadOnDemand = false;
}

CGUIWindowHome::~CGUIWindowHome(void)
{
}

void CGUIWindowHome::Render()
{
	//TEST 
	//===============================================

#if 0
	MEMORYSTATUS stat;
	CHAR strOut[1024], *pstrOut;

	// Get the memory status.
	GlobalMemoryStatus( &stat );

	// Setup the output string.
	pstrOut = strOut;

	CStdString strOut1;

	strOut1.Format("fps:%02.2f", g_infoManager.GetFPS() );

	AddStr( "%4d  free MB of physical memory.\n", stat.dwAvailPhys / MB );

	CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), 88/*ID*/);
	msg.SetLabel(strOut1);
	OnMessage(msg);

#endif

	//TEST 
	//===============================================

	CGUIWindow::Render();
}
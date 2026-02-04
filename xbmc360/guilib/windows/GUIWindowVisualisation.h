#pragma once

#include "guilib\GUIWindow.h"

class CGUIWindowVisualisation :
	public CGUIWindow
{
public:
	CGUIWindowVisualisation(void);
	virtual ~CGUIWindowVisualisation(void);
	virtual void AllocResources(bool forceLoad = false);
	virtual void FreeResources(bool forceUnLoad = false);
	virtual bool OnMessage(CGUIMessage& message);
	virtual bool OnAction(const CAction &action);
	virtual void FrameMove();

protected:
	DWORD m_dwInitTimer;
	DWORD m_dwLockedTimer;
	bool m_bShowPreset;
};

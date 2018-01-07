#ifndef GUILIB_CGUIDIALOG_H
#define GUILIB_CGUIDIALOG_H

#include "GUIWindow.h"

class CGUIDialog : public CGUIWindow
{
public:
	CGUIDialog(int id, const CStdString &xmlFile);
	virtual ~CGUIDialog(void);

	virtual bool OnMessage(CGUIMessage& message);
	virtual void Render();

	virtual bool IsDialog() const { return true; };
	virtual bool IsRunning() const { return m_bRunning; }
	void DoModal(DWORD dwParentId, int iWindowID = WINDOW_INVALID); // modal
	virtual void Close(bool forceClose = false);

protected:
	bool m_bRunning;
};

#endif //GUILIB_CGUIDIALOG_H
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

	virtual void DoModal(int iWindowID = WINDOW_INVALID); // Modal
	void Show(); // Modeless

	virtual bool OnBack(int actionID);
	virtual void Close(bool forceClose = false);
	virtual bool IsModalDialog() const { return m_bModal; };

protected:
	bool m_bRunning;
	bool m_bModal;
};

#endif //GUILIB_CGUIDIALOG_H
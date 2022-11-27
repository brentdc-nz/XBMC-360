#ifndef GUILIB_CGUIDIALOG_H
#define GUILIB_CGUIDIALOG_H

#include "guilib\GUIWindow.h"

class CGUIDialog : public CGUIWindow
{
public:
	CGUIDialog(int id, const CStdString &xmlFile);
	virtual ~CGUIDialog(void);

	virtual bool OnAction(const CAction &action);
	virtual bool OnMessage(CGUIMessage& message);
	virtual void FrameMove();
	virtual void Render();

	virtual void DoModal(int iWindowID = WINDOW_INVALID, const CStdString &param = ""); // Modal
	void DoModalThreadSafe(); // Threadsafe version of DoModal
	void Show(); // Modeless
  
	virtual bool OnBack(int actionID);

	virtual void Close(bool forceClose = false);
	virtual bool IsDialogRunning() const { return m_bRunning; };
	virtual bool IsDialog() const { return true;};
	virtual bool IsModalDialog() const { return m_bModal; };

	virtual bool IsAnimating(ANIMATION_TYPE animType);

	void SetAutoClose(unsigned int timeoutMs);
	void SetSound(bool OnOff) { m_enableSound = OnOff; };

protected:
	virtual bool RenderAnimation(unsigned int time);
	virtual void SetDefaults();
	virtual void OnWindowLoaded();

	void DoModal_Internal(int iWindowID = WINDOW_INVALID, const CStdString &param = ""); // Modal
	void Show_Internal(); // Modeless

	bool m_bRunning;
	bool m_bModal;
	bool m_dialogClosing;
	bool m_autoClosing;
	bool m_enableSound;
	unsigned int m_showStartTime;
	unsigned int m_showDuration;
};

#endif //GUILIB_CGUIDIALOG_H
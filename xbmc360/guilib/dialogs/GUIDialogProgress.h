#ifndef GUILIB_CGUIDIALOGGPROGRESS_H
#define GUILIB_CGUIDIALOGGPROGRESS_H

#include "guilib\GUIDialogBoxBase.h"
#include "IProgressCallback.h"

class CGUIDialogProgress : public CGUIDialogBoxBase, public IProgressCallback
{
public:
	CGUIDialogProgress(void);
	virtual ~CGUIDialogProgress(void);

	virtual void DoModal(int iWindowID = WINDOW_INVALID, const CStdString &param = ""); // Override that just starts the window up. Needs Close() to close it
	void StartModal(bool threadSafe = false); // Thread safe routine wrapping DoModal() if set to true
	virtual bool OnMessage(CGUIMessage& message);
	virtual bool OnBack(int actionID);
	virtual void OnWindowLoaded();
	void Progress();
	void ProgressKeys();
	bool IsCanceled() const { return m_bCanceled; }
	void SetPercentage(int iPercentage);
	int GetPercentage() const { return m_percentage; };
	void ShowProgressBar(bool bOnOff);
	void SetHeading(const std::string& strLine);
	void SetHeading(int iString); // For convenience to lookup in strings.xml

	// Implements IProgressCallback
	virtual void SetProgressMax(int iMax);
	virtual void SetProgressAdvance(int nSteps=1);
	virtual bool Abort();

	void SetCanCancel(bool bCanCancel);

protected:
	bool m_bCanCancel;
	bool m_bCanceled;
	std::string m_strHeading;

	int  m_iCurrent;
	int  m_iMax;
	int m_percentage;
};

#endif //GUILIB_CGUIDIALOGGPROGRESS_H
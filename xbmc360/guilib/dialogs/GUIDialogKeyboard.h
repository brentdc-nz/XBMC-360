#ifndef GUILIB_CGUIDIALOGGKEYBOARDS_H
#define GUILIB_CGUIDIALOGGKEYBOARDS_H

#include "guilib\GUIDialog.h"

enum KEYBOARD {CAPS, LOWER, SYMBOLS };

class CGUIDialogKeyboard: public CGUIDialog
{
public:
	CGUIDialogKeyboard(void);
	virtual ~CGUIDialogKeyboard(void);

	virtual void FrameMove();
	void SetHeading(const CStdString& strHeading) {m_strHeading = strHeading;} ;
	void SetText(const CStdString& aTextString);
	CStdString GetText() const;
	bool IsConfirmed() { return m_bIsConfirmed; };
	void SetHiddenInput(bool hiddenInput) { m_hiddenInput = hiddenInput; };

	static bool ShowAndGetInput(CStdString& aTextString, bool allowEmptyResult);
	static bool ShowAndGetInput(CStdString& aTextString, const CStdString &strHeading, bool allowEmptyResult, bool hiddenInput = false);
	static bool ShowAndGetNewPassword(CStdString& strNewPassword);
	static bool ShowAndGetNewPassword(CStdString& newPassword, const CStdString &heading, bool allowEmpty);
	static bool ShowAndVerifyNewPassword(CStdString& strNewPassword);
	static bool ShowAndVerifyNewPassword(CStdString& newPassword, const CStdString &heading, bool allowEmpty);
	static int ShowAndVerifyPassword(CStdString& strPassword, const CStdString& strHeading, int iRetries);
	static bool ShowAndGetFilter(CStdString& aTextString, bool searching);

	virtual void Close(bool forceClose = false);

protected:
	virtual void OnInitWindow();
	virtual bool OnAction(const CAction &action);
	virtual bool OnMessage(CGUIMessage& message);
	void SetControlLabel(int id, const CStdString &label);
	void OnShift();
	void MoveCursor(int iAmount);
	int GetCursorPos() const;
	void OnSymbols();
	void OnIPAddress();
	void OnOK();

private:
	void OnClickButton(int iButtonControl);
	void OnRemoteNumberClick(int key);
	void UpdateButtons();
	char GetCharacter(int iButton);
	void UpdateLabel();
	void ResetShiftAndSymbols();

	void Character(WCHAR wch);
	void Backspace();
	void SendSearchMessage();

	CStdStringW m_strEdit;
	bool m_bIsConfirmed;
	KEYBOARD m_keyType;
	int m_iMode;
	bool m_bShift;
	bool m_hiddenInput;

	// Filtering type
	enum FILTERING { FILTERING_NONE = 0, FILTERING_CURRENT, FILTERING_SEARCH };
	FILTERING m_filtering;

	DWORD m_lastRemoteClickTime;
	WORD m_lastRemoteKeyClicked;
	int m_indexInSeries;
	CStdString m_strHeading;
	static const char* s_charsSeries[10];
};

#endif //GUILIB_CGUIDIALOGGKEYBOARDS_H
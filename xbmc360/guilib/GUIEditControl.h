#ifndef GUILIB_GUIEditControl_H
#define GUILIB_GUIEditControl_H

#include "GUIButtonControl.h"

class CGUIEditControl : public CGUIButtonControl
{
public:
	enum INPUT_TYPE {
					INPUT_TYPE_TEXT = 0,
					INPUT_TYPE_NUMBER,
					INPUT_TYPE_SECONDS,
					INPUT_TYPE_DATE,
					INPUT_TYPE_IPADDRESS,
					INPUT_TYPE_PASSWORD,
					INPUT_TYPE_PASSWORD_MD5,
					INPUT_TYPE_SEARCH,
					INPUT_TYPE_FILTER
					};

	CGUIEditControl(int parentID, int controlID, float posX, float posY,
					float width, float height, const CTextureInfo &textureFocus, const CTextureInfo &textureNoFocus,
					const CLabelInfo& labelInfo, const std::string &text);
	CGUIEditControl(const CGUIButtonControl &button);
	virtual ~CGUIEditControl(void);
	virtual CGUIEditControl *Clone() const { return new CGUIEditControl(*this); };

	virtual bool OnMessage(CGUIMessage &message);
	virtual bool OnAction(const CAction &action);
	virtual void OnClick();

	virtual void SetLabel(const std::string &text);
	virtual void SetLabel2(const std::string &text);
	void SetHint(const CGUIInfoLabel& hint);

	virtual CStdString GetLabel2() const;

	unsigned int GetCursorPosition() const;
	void SetCursorPosition(unsigned int iPosition);

	void SetInputType(INPUT_TYPE type, int heading);
 
	void SetTextChangeActions(const CGUIAction& textChangeActions) { m_textChangeActions = textChangeActions; };
  
	bool HasTextChangeActions() { return m_textChangeActions.HasActionsMeetingCondition(); };

protected:
	virtual void RenderText();
	CStdStringW GetDisplayedText() const;
	void RecalcLabelPosition();
	void ValidateCursor();
	void OnPasteClipboard();
	void OnTextChanged();
	void DefaultConstructor();

	// Clear out the current text input if it's an MD5 password.
	// return true if the password is cleared, false otherwise
	bool ClearMD5();

	CStdStringW m_text2;
	CStdString  m_text;
	CGUIInfoLabel m_hintInfo;
	float m_textOffset;
	float m_textWidth;

	static const int spaceWidth = 5;

	unsigned int m_cursorPos;
	unsigned int m_cursorBlink;

	int m_inputHeading;
	INPUT_TYPE m_inputType;
	bool m_isMD5;
  
	CGUIAction m_textChangeActions;
};

#endif //GUILIB_GUIEditControl_H
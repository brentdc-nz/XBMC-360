#ifndef CGUIDIALOGNETWORKSETUP_H
#define CGUIDIALOGNETWORKSETUP_H

#include "guilib\GUIDialog.h"

class CGUIDialogNetworkSetup :
	public CGUIDialog
{
public:
	enum NET_PROTOCOL { NET_PROTOCOL_SMB = 0}; // TODO - Add more!
	
	CGUIDialogNetworkSetup(void);
	virtual ~CGUIDialogNetworkSetup(void);
	virtual bool OnMessage(CGUIMessage& message);
	virtual bool OnBack(int actionID);
	virtual void OnInitWindow();

	static bool ShowAndGetNetworkAddress(CStdString &path);

	CStdString ConstructPath() const;
	void SetPath(const CStdString &path);
	bool IsConfirmed() const { return m_confirmed; };

protected:
	void OnProtocolChange();
	void OnServerBrowse();
	void OnOK();
	void OnCancel();
	void UpdateButtons();

	NET_PROTOCOL m_protocol;
	CStdString m_server;
	CStdString m_path;
	CStdString m_username;
	CStdString m_password;
	CStdString m_port;

	bool m_confirmed;
};

#endif //CGUIDIALOGNETWORKSETUP_H
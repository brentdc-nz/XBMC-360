#ifndef GUILIB_CGUIDIALOGNETWORKSETUP_H
#define GUILIB_CGUIDIALOGNETWORKSETUP_H

#include "..\GUIDialog.h"

class CGUIDialogNetworkSetup : public CGUIDialog
{
public:
	enum NET_PROTOCOL { NET_PROTOCOL_SMB = 0, //TODO: Only have SMB atm
                        NET_PROTOCOL_FTP};

	CGUIDialogNetworkSetup();
	virtual ~CGUIDialogNetworkSetup();

	virtual void OnInitWindow();
	virtual bool OnMessage(CGUIMessage& message);
	static bool ShowAndGetNetworkAddress(CStdString &path);
	bool IsConfirmed() const { return m_confirmed; };
	void SetPath(const CStdString &path);
	CStdString ConstructPath() const;

protected:
	void OnProtocolChange();
	void OnServerAddress();
	void OnPath();
	void OnPort();
	void OnUserName();
	void OnPassword();
	void OnOK();
	void OnCancel();
	void UpdateButtons();

	bool m_confirmed;
	CStdString m_server;
	CStdString m_path;
	CStdString m_username;
	CStdString m_password;
	CStdString m_port;
	NET_PROTOCOL m_protocol;
};

#endif //GUILIB_CGUIDIALOGNETWORKSETUP_H
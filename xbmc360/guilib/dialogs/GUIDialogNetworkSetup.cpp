#include "GUIDialogNetworkSetup.h"
#include "..\GUIWindowManager.h"
#include "..\GUISpinControlEx.h"
#include "..\LocalizeStrings.h"
#include "..\GUIMessage.h"
#include "..\..\URL.h"
#include "..\..\utils\Util.h"

#define CONTROL_PROTOCOL        10
#define CONTROL_SERVER_ADDRESS  11
#define CONTROL_PORT_NUMBER     13
#define CONTROL_USERNAME        14
#define CONTROL_PASSWORD        15
#define CONTROL_REMOTE_PATH     16
#define CONTROL_OK              18
#define CONTROL_CANCEL          19

CGUIDialogNetworkSetup::CGUIDialogNetworkSetup(void)
	: CGUIDialog(WINDOW_DIALOG_NETWORK_SETUP, "DialogNetworkSetup.xml")
{
	m_confirmed = false;
	m_protocol = NET_PROTOCOL_SMB;
}

CGUIDialogNetworkSetup::~CGUIDialogNetworkSetup()
{
}

void CGUIDialogNetworkSetup::OnInitWindow()
{
	CGUIDialog::OnInitWindow();

	// Add our protocols
	CGUISpinControlEx *pSpin = (CGUISpinControlEx *)GetControl(CONTROL_PROTOCOL);
	if(!pSpin)
		return;

	pSpin->Clear();
	pSpin->AddLabel(/*g_localizeStrings.Get(20171)*/"SMB", NET_PROTOCOL_SMB);





	m_server = "192.168.1.196";// TEST - Remove ME
	m_port = "445"; // TEST - Remove ME
	m_path = "Storage"; // TEST - Remove ME
	m_username = "Administrator";
	m_password = "passwordxyz";

	UpdateButtons();// TEST - Remove ME



	pSpin->SetValue(m_protocol);
}

bool CGUIDialogNetworkSetup::OnMessage(CGUIMessage& message)
{
	switch( message.GetMessage())
	{
		case GUI_MSG_CLICKED:
		{
			int iControl = message.GetSenderId();
			if(iControl == CONTROL_PROTOCOL)
			{
				m_server.Empty();
				m_path.Empty();
				m_username.Empty();
				m_password.Empty();
				OnProtocolChange();
			}
			else if (iControl == CONTROL_SERVER_ADDRESS)
				OnServerAddress();
			else if (iControl == CONTROL_REMOTE_PATH)
				OnPath();
			else if (iControl == CONTROL_PORT_NUMBER)
				OnPort();
			else if (iControl == CONTROL_USERNAME)
				OnUserName();
			else if (iControl == CONTROL_PASSWORD)
				OnPassword();
			else if (iControl == CONTROL_OK)
				OnOK();
			else if (iControl == CONTROL_CANCEL)
				OnCancel();
			
			return true;
		}
		break;
	}
	return CGUIDialog::OnMessage(message);
}

// Show CGUIDialogNetworkSetup dialog and prompt for a new network address,
// return True if the network address is valid, false otherwise
bool CGUIDialogNetworkSetup::ShowAndGetNetworkAddress(CStdString &path)
{
	CGUIDialogNetworkSetup *dialog = (CGUIDialogNetworkSetup *)g_windowManager.GetWindow(WINDOW_DIALOG_NETWORK_SETUP);
	if(!dialog) return false;

	dialog->Initialize();
	dialog->SetPath(path);
	dialog->DoModal();
	path = dialog->ConstructPath();

	return dialog->IsConfirmed();
}


void CGUIDialogNetworkSetup::OnServerAddress()
{
//	CGUIDialogKeyboard::ShowAndGetInput(m_server, g_localizeStrings.Get(1016), false); // TODO

	UpdateButtons();
}

void CGUIDialogNetworkSetup::OnPath()
{
//	CGUIDialogKeyboard::ShowAndGetInput(m_path, g_localizeStrings.Get(1017), false); // TODO
	UpdateButtons();
}

void CGUIDialogNetworkSetup::OnPort()
{
//	CGUIDialogNumeric::ShowAndGetNumber(m_port, g_localizeStrings.Get(1018)); // TODO
	UpdateButtons();
}

void CGUIDialogNetworkSetup::OnUserName()
{
//	CGUIDialogKeyboard::ShowAndGetInput(m_username, g_localizeStrings.Get(1019), false); // TODO
	UpdateButtons();
}

void CGUIDialogNetworkSetup::OnPassword()
{
//	CGUIDialogKeyboard::ShowAndGetNewPassword(m_password,g_localizeStrings.Get(12326),true); // TODO
	UpdateButtons();
}

void CGUIDialogNetworkSetup::OnOK()
{
	m_confirmed = true;
	Close();
}

void CGUIDialogNetworkSetup::OnCancel()
{
	m_confirmed = false;
	Close();
}

void CGUIDialogNetworkSetup::OnProtocolChange()
{
}

void CGUIDialogNetworkSetup::UpdateButtons()
{
	// Button labels
	if(m_protocol == NET_PROTOCOL_SMB)
	{
		SET_CONTROL_LABEL(CONTROL_SERVER_ADDRESS, /*1010*/"Server Name"); // Server name
	}
	else
	{
		SET_CONTROL_LABEL(CONTROL_SERVER_ADDRESS, /*1009*/"Server Address"); // Server Address
	}

	// Server
	CGUIButtonControl *server = (CGUIButtonControl *)GetControl(CONTROL_SERVER_ADDRESS);
	if(server)
		server->SetLabel2(m_server);

	// Remote path
	CGUIButtonControl *path = (CGUIButtonControl *)GetControl(CONTROL_REMOTE_PATH);
	if(path)
		path->SetLabel2(m_path);

	// Port
	CGUIButtonControl *port = (CGUIButtonControl *)GetControl(CONTROL_PORT_NUMBER);
	if(port)
		port->SetLabel2(m_port);

	// Username
	CGUIButtonControl *username = (CGUIButtonControl *)GetControl(CONTROL_USERNAME);
	if(username)
		username->SetLabel2(m_username);

	// Password
	CGUIButtonControl *password = (CGUIButtonControl *)GetControl(CONTROL_PASSWORD);
	if(password)
	{
		CStdString asterix;
		asterix.append(m_password.size(), '*');
		password->SetLabel2(asterix);
	}
}

void CGUIDialogNetworkSetup::SetPath(const CStdString &path)
{
	CURL url(path);
		const CStdString &protocol = url.GetProtocol();
	if (protocol == "smb")
		m_protocol = NET_PROTOCOL_SMB;
	else if (protocol == "ftp")
		m_protocol = NET_PROTOCOL_FTP;
	else
		m_protocol = NET_PROTOCOL_SMB;  // Default to SMB

//	m_username = url.GetUserName();
//	m_password = url.GetPassWord();
//	m_port.Format("%i", url.GetPort());
//	m_server = url.GetHostName();
	m_path = url.GetFileName();
}

CStdString CGUIDialogNetworkSetup::ConstructPath() const
{
	CStdString path;
	
	if(m_protocol == NET_PROTOCOL_SMB)
		path = "smb://";
	else if(m_protocol == NET_PROTOCOL_FTP)
		path = "ftp://";

	if(!m_username.IsEmpty())
	{
		path += m_username;
		if(!m_password.IsEmpty())
		{
			path += ":";
			path += m_password;
		}
		path += "@";
	}

	path += m_server;
	if((m_protocol == NET_PROTOCOL_SMB && !m_port.IsEmpty() && atoi(m_port.c_str()) > 0))
	{
		path += ":";
		path += m_port;
	}
	
	if(!m_path.IsEmpty())
		CUtil::AddFileToFolder(path, m_path, path);

	CUtil::AddSlashAtEnd(path);
	return path;
}
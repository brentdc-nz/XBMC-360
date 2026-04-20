#include "FileFactory.h"
#include "Application.h"
#include "utils\Log.h"

#include "FileHD.h"
#include "FileSMB.h"
#include "CurlFile.h"
#include "UPnPFile.h"
using namespace XFILE;

CFileFactory::CFileFactory()
{
}

CFileFactory::~CFileFactory()
{
}

CFileBase* CFileFactory::CreateLoader(const CStdString& strFileName)
{
	CURL url(strFileName);
	return CreateLoader(url);
}

CFileBase* CFileFactory::CreateLoader(const CURL& url)
{
	CStdString strProtocol = url.GetProtocol();
	strProtocol.MakeLower();

	if(strProtocol == "file" || strProtocol.IsEmpty()) return new CFileHD();

	if(g_application.getNetwork().IsAvailable())
	{
		if(strProtocol == "smb") return new CFileSMB();
		if(strProtocol == "http" || strProtocol == "https") return new CCurlFile();
		if(strProtocol == "upnp") return new CUPnPFile();
	}

	CLog::Log(LOGWARNING, "%s - Unsupported protocol(%s) in %s", __FUNCTION__, strProtocol.c_str(), ""/*url.Get().c_str()*/ ); // TODO

	return NULL;
}
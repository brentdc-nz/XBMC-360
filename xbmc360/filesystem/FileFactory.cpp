#include "FileFactory.h"
#include "Application.h"
#include "utils\Log.h"

#include "FileHD.h"
#include "FileSMB.h"
#include "CurlFile.h"
#include "UPnPFile.h"
#include "ZipFile.h"
#include "SpecialProtocolFile.h"

using namespace XFILE;

CFileFactory::CFileFactory()
{
}

CFileFactory::~CFileFactory()
{
}

IFile* CFileFactory::CreateLoader(const CStdString& strFileName)
{
	CURL url(strFileName);
	return CreateLoader(url);
}

IFile* CFileFactory::CreateLoader(const CURL& url)
{
	CStdString strProtocol = url.GetProtocol();
	strProtocol.MakeLower();

	if (strProtocol == "zip") return new CZipFile();
//	if (strProtocol == "rar") return new CRarFile(); // TODO

	if (strProtocol == "special") return new CSpecialProtocolFile();

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
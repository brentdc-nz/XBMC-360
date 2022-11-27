#include "FileFactory.h"
#include "Application.h"
#include "utils\Log.h"

#include "FileHD.h"
#include "FileSMB.h"

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

		// TODO: Add more types, ftp, etc
	}

	CLog::Log(LOGWARNING, "%s - Unsupported protocol(%s) in %s", __FUNCTION__, strProtocol.c_str(), ""/*url.Get().c_str()*/ ); // TODO

	return NULL;
}
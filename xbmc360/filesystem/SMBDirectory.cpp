#include "SMBDirectory.h"
#include "utils\SingleLock.h"
#include "utils\Log.h"
#include "utils\Util.h"
#include "libraries\libsmb2\XBLibSmb2.h"
#include "FileItem.h"

using namespace XFILE;

CXBLibSMB2 xbsmb;

CSMBDirectory::CSMBDirectory(void)
{
} 

CSMBDirectory::~CSMBDirectory(void)
{
}

bool CSMBDirectory::GetDirectory(const CStdString& strPath, CFileItemList &items) // TODO: Missing the file stats stuff!
{
	// We accept smb://[[[domain;]user[:password@]]server[/share[/path[/file]]]]

	// LibSMB2 isn't thread safe, always lock
	CSingleLock lock(xbsmb);

	// We need an url to do proper escaping
	CURL url(strPath);

	xbsmb.Init();

	// Separate roots for the authentication and the containing items to allow browsing to work correctly
	CStdString strRoot = strPath;

	if(!xbsmb.OpenDir(url))
		return false;

	struct smb2dirent* dirEnt = NULL;
	CStdString strFile;

	if(!CUtil::HasSlashAtEnd(strRoot)) strRoot += "/";

	while(dirEnt = xbsmb.ReadDir())
	{
		strFile = dirEnt->name;

		if(strFile == ".." || strFile == ".")
			continue;

		if(dirEnt->st.smb2_type == SMB2_TYPE_DIRECTORY)
		{
			CFileItemPtr pItem(new CFileItem(strFile));
			pItem->m_strPath = strRoot;
/*
			// Needed for network / workgroup browsing
			// skip if root if we are given a server
			if(dirEnt->smbc_type == SMBC_SERVER)
			{
				// Create url with same options, user, pass.. but no filename or host
				CURL rooturl(strRoot);
				rooturl.SetFileName("");
				rooturl.SetHostName("");
				pItem->m_strPath = smb.URLEncode(rooturl);
			}
*/
			pItem->m_strPath += dirEnt->name;

			if(!CUtil::HasSlashAtEnd(pItem->m_strPath))
				pItem->m_strPath += '/';

			pItem->m_bIsFolder = true;
//			pItem->m_dateTime = localTime;
//			vecCacheItems.Add(pItem);
			items.Add(pItem);
		}
		else if(dirEnt->st.smb2_type == SMB2_TYPE_FILE)
		{
			CFileItemPtr pItem(new CFileItem(strFile));
			pItem->m_strPath = strRoot + dirEnt->name;
			pItem->m_bIsFolder = false;
//			pItem->m_dwSize = iSize;
//			pItem->m_dateTime = localTime;

//			vecCacheItems.Add(pItem);
			if(IsAllowed(dirEnt->name))
				items.Add(pItem);
		}
	}

	xbsmb.Close();

	return true;
}

bool CSMBDirectory::Create(const char* strPath)
{
	return false;
}

bool CSMBDirectory::Remove(const char* strPath)
{
	return false;
}

bool CSMBDirectory::Exists(const char* strPath)
{
	return true;
}
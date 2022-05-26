#include "Directory.h"
#include "IDirectory.h"
#include "FactoryDirectory.h"
#include "..\utils\log.h"

using namespace DIRECTORY;

CDirectory::CDirectory()
{
}

CDirectory::~CDirectory()
{
}

bool CDirectory::GetDirectory(const CStdString& strPath, CFileItemList &items, CStdString strMask /*=""*/, bool bUseFileDirectories /* = true */, bool allowPrompting /* = false */, bool cacheDirectory /* = false */)
{
	try 
	{
		CStdString translatedPath(strPath);
//		if(strPath.Left(10) == "special://")
//		{
//			// Need to translate this special folder
//			translatedPath = CUtil::TranslateSpecialPath(strPath);
//		}

		auto_ptr<IDirectory> pDirectory(CFactoryDirectory::Create(translatedPath));

		if(!pDirectory.get())
			return false;
	
		pDirectory->SetMask(strMask);
//		pDirectory->SetAllowPrompting(allowPrompting);
//		pDirectory->SetCacheDirectory(cacheDirectory);

		items.SetPath(strPath);

		bool bSuccess = pDirectory->GetDirectory(translatedPath, items);
		if(bSuccess)
		{
/*			// Should any of the files we read be treated as a directory?
			// Disable for musicdatabase, it already contains the extracted items
			if(bUseFileDirectories && !items.IsMusicDb())
				for(int i=0; i< items.Size(); ++i)
				{
					CFileItem* pItem=items[i];
					if((!pItem->m_bIsFolder) && (!pItem->IsInternetStream()))
					{
						auto_ptr<IFileDirectory> pDirectory(CFactoryFileDirectory::Create(pItem->m_strPath,pItem,strMask));
						if(pDirectory.get())
							pItem->m_bIsFolder = true;
						else
							if(pItem->m_bIsFolder)
							{
								items.Remove(i);
								i--; // Don't confuse loop
							}
					}
				}
*/		}
		return bSuccess;
	}
	catch (...) 
	{
		CLog::Log(LOGERROR, __FUNCTION__" - Unhandled exception");    
	}

	CLog::Log(LOGERROR, __FUNCTION__" - Error getting %s", strPath.c_str());  	
	return false;
}
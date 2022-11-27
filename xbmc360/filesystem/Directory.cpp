#include "utils\Log.h"
#include "Directory.h"
#include "DirectoryFactory.h"
#include "FileDirectoryFactory.h"
#include "utils\Win32Exception.h"
#include "FileItem.h"
#include "DirectoryCache.h"
#include "Settings.h"

using namespace std;
using namespace XFILE;

CDirectory::CDirectory()
{
}

CDirectory::~CDirectory()
{
}

bool CDirectory::GetDirectory(const CStdString& strPath, CFileItemList &items, CStdString strMask /*=""*/, bool bUseFileDirectories /* = true */, bool allowPrompting /* = false */, DIR_CACHE_TYPE cacheDirectory /* = DIR_CACHE_ONCE */, bool extFileInfo /* = true */)
{
	try
	{
		auto_ptr<IDirectory> pDirectory(CFactoryDirectory::Create(strPath));
		if (!pDirectory.get())
			return false;

		// Check our cache for this path
		if (g_directoryCache.GetDirectory(strPath, items, cacheDirectory == DIR_CACHE_ALWAYS))
			items.SetPath(strPath);
		else
		{
			// Need to clear the cache (in case the directory fetch fails)
			// and (re)fetch the folder
			if (cacheDirectory != DIR_CACHE_NEVER)
				g_directoryCache.ClearDirectory(strPath);

			pDirectory->SetAllowPrompting(allowPrompting);
			pDirectory->SetCacheDirectory(cacheDirectory);
			pDirectory->SetUseFileDirectories(bUseFileDirectories);
			pDirectory->SetExtFileInfo(extFileInfo);

			items.SetPath(strPath);

			if (!pDirectory->GetDirectory(strPath, items))
			{
				CLog::Log(LOGERROR, "%s - Error getting %s", __FUNCTION__, strPath.c_str());
				return false;
			}

			// Cache the directory, if necessary
			if (cacheDirectory != DIR_CACHE_NEVER)
				g_directoryCache.SetDirectory(strPath, items, pDirectory->GetCacheType(strPath));
		}

		// Now filter for allowed files
		pDirectory->SetMask(strMask);
		
		for (int i = 0; i < items.Size(); ++i)
		{
			CFileItemPtr item = items[i];
			if ((!item->m_bIsFolder && !pDirectory->IsAllowed(item->GetPath()))/* ||
				(item->GetPropertyBOOL("file:hidden") && !g_guiSettings.GetBool("filelists.showhidden"))*/)// TODO
			{
				items.Remove(i);
				i--; // Don't confuse loop
			}
		}

		//  Should any of the files we read be treated as a directory?
		//  Disable for database folders, as they already contain the extracted items
//		if (bUseFileDirectories && !items.IsMusicDb() && !items.IsVideoDb() && !items.IsSmartPlayList())// TODO
//			FilterFileDirectories(items, strMask);// TODO

		return true;
	}
	catch (const win32_exception &e)
	{
		e.writelog(__FUNCTION__);
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s - Unhandled exception", __FUNCTION__);
	}
	
	CLog::Log(LOGERROR, "%s - Error getting %s", __FUNCTION__, strPath.c_str());
	return false;
}

bool CDirectory::Create(const CStdString& strPath)
{
	try
	{
		auto_ptr<IDirectory> pDirectory(CFactoryDirectory::Create(strPath));
		
		if (pDirectory.get())
			if(pDirectory->Create(strPath.c_str()))
				return true;
	}
	catch (const win32_exception &e)
	{
		e.writelog(__FUNCTION__);
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s - Unhandled exception", __FUNCTION__);
	}

	CLog::Log(LOGERROR, "%s - Error creating %s", __FUNCTION__, strPath.c_str());
	return false;
}

bool CDirectory::Exists(const CStdString& strPath)
{
	try
	{
		auto_ptr<IDirectory> pDirectory(CFactoryDirectory::Create(strPath));
		
		if (pDirectory.get())
			return pDirectory->Exists(strPath.c_str());
	}
	catch (const win32_exception &e)
	{
		e.writelog(__FUNCTION__);
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s - Unhandled exception", __FUNCTION__);
	}
	
	CLog::Log(LOGERROR, "%s - Error checking for %s", __FUNCTION__, strPath.c_str());
	return false;
}

bool CDirectory::Remove(const CStdString& strPath)
{
	try
	{
		auto_ptr<IDirectory> pDirectory(CFactoryDirectory::Create(strPath));
		
		if (pDirectory.get())
			if(pDirectory->Remove(strPath.c_str()))
				return true;
	}
	catch (const win32_exception &e)
	{
		e.writelog(__FUNCTION__);
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s - Unhandled exception", __FUNCTION__);
	}
	
	CLog::Log(LOGERROR, "%s - Error removing %s", __FUNCTION__, strPath.c_str());
	return false;
}

void CDirectory::FilterFileDirectories(CFileItemList &items, const CStdString &mask)
{
	for (int i=0; i< items.Size(); ++i)
	{
		CFileItemPtr pItem=items[i];
		if ((!pItem->m_bIsFolder) && (!pItem->IsInternetStream()))// TODO
		{
			auto_ptr<IFileDirectory> pDirectory(CFactoryFileDirectory::Create(pItem->GetPath(),pItem.get(),mask));
			
			if (pDirectory.get())
				pItem->m_bIsFolder = true;
			else
			{
				if (pItem->m_bIsFolder)
				{
					items.Remove(i);
					i--; // Don't confuse loop
				}
			}
		}
	}
}
//#include "system.h" // TODO
#include "utils\Util.h"
#include "utils/\URIUtils.h"
#include "FileDirectoryFactory.h"
#ifdef HAS_FILESYSTEM
#include "OGGFileDirectory.h"
#include "NSFFileDirectory.h"
#include "SIDFileDirectory.h"
#include "ASAPFileDirectory.h"
#include "cores/paplayer/ASAPCodec.h"
#endif
//#include "RarDirectory.h" // TODO
//#include "ZipDirectory.h" // TODO
//#include "SmartPlaylistDirectory.h" // TODO
//#include "SmartPlaylist.h" // TODO
//#include "PlaylistFileDirectory.h" // TODO
//#include "playlists/PlayListFactory.h" // TODO
#include "FileSystem/Directory.h"
#include "FileSystem/File.h"
//#include "FileSystem/RarManager.h" // TODO
//#include "FileSystem/ZipManager.h" // TODO
//#include "settings/AdvancedSettings.h" // TODO
#include "GUISettings.h"
#include "FileItem.h"

using namespace XFILE;
//using namespace PLAYLIST; // TODO
using namespace std;

CFactoryFileDirectory::CFactoryFileDirectory(void)
{
}

CFactoryFileDirectory::~CFactoryFileDirectory(void)
{
}

// Return NULL + set pItem->m_bIsFolder to remove it completely from list.
IFileDirectory* CFactoryFileDirectory::Create(const CStdString& strPath, CFileItem* pItem, const CStdString& strMask)
{
	CStdString strExtension=URIUtils::GetExtension(strPath);
	strExtension.MakeLower();

#ifdef HAS_FILESYSTEM
	if ((strExtension.Equals(".ogg") || strExtension.Equals(".oga")) && CFile::Exists(strPath))
	{
		IFileDirectory* pDir=new COGGFileDirectory;
		// Has the ogg file more than one bitstream?
		if (pDir->ContainsFiles(strPath))
		{
			return pDir; // treat as directory
		}

		delete pDir;
		return NULL;
	}
	
	if (strExtension.Equals(".nsf") && CFile::Exists(strPath))
	{
		IFileDirectory* pDir = new CNSFFileDirectory;

		//  Has the nsf file more than one track?
		if (pDir->ContainsFiles(strPath))
			return pDir; // treat as directory

		delete pDir;
		return NULL;
	}
	
	if (strExtension.Equals(".sid") && CFile::Exists(strPath))
	{
		IFileDirectory* pDir = new CSIDFileDirectory;
		
		//  Has the sid file more than one track?
		if (pDir->ContainsFiles(strPath))
			return pDir; // Treat as directory

		delete pDir;
		return NULL;
	}
	
	if (ASAPCodec::IsSupportedFormat(strExtension) && CFile::Exists(strPath))
	{
		IFileDirectory* pDir=new CASAPFileDirectory;
		
		//  Has the asap file more than one track?
		if (pDir->ContainsFiles(strPath))
			return pDir; // treat as directory

		delete pDir;
		return NULL;
	}
#endif
/*	if (strExtension.Equals(".zip")) // TODO
	{
		CStdString strUrl; 
		URIUtils::CreateArchivePath(strUrl, "zip", strPath, "");

		if (!g_guiSettings.GetBool("filelists.unrollarchives"))
		{
			pItem->SetPath(strUrl);
			return new CZipDirectory;
		}

		CFileItemList items;
		CDirectory::GetDirectory(strUrl, items, strMask);
		
		if (items.Size() == 0) // No files
			pItem->m_bIsFolder = true;
		else if (items.Size() == 1 && items[0]->m_idepth == 0) 
		{
			// One STORED file - collapse it down
			*pItem = *items[0]; 
		}
		else
		{
			// Compressed or more than one file -> create a zip dir
			pItem->SetPath(strUrl);
			return new CZipDirectory;
		}
		return NULL;
	}
*/	
	if (strExtension.Equals(".rar") || strExtension.Equals(".001")) // TODO
	{
		CStdString strUrl; 
/*		URIUtils::CreateArchivePath(strUrl, "rar", strPath, "");

		vector<CStdString> tokens;
		CUtil::Tokenize(strPath,tokens,".");
		
		if (tokens.size() > 2)
		{
			if (strExtension.Equals(".001"))
			{
				if (tokens[tokens.size()-2].Equals("ts")) // .ts.001 - treat as a movie file to scratch some users itch
					return NULL;
			}
			
			CStdString token = tokens[tokens.size()-2];
			if (token.Left(4).CompareNoCase("part") == 0) // Only list '.part01.rar'
			{
				// Need this crap to avoid making mistakes - yeyh for the new rar naming scheme :/
				__stat64 stat;
				int digits = token.size()-4;
				CStdString strNumber, strFormat;
				strFormat.Format("part%%0%ii",digits);
				strNumber.Format(strFormat.c_str(),1);
				CStdString strPath2=strPath;
				strPath2.Replace(token,strNumber);
				
				if (atoi(token.substr(4).c_str()) > 1 && CFile::Stat(strPath2,&stat) == 0)
				{
					pItem->m_bIsFolder = true;
					return NULL;
				}
			}
		}
 */  
/*		if (!g_guiSettings.GetBool("filelists.unrollarchives")) // TODO
		{
			pItem->SetPath(strUrl);
			return new CRarDirectory;
		}
*/
		CFileItemList items;
		CDirectory::GetDirectory(strUrl, items, strMask);
		
		if (items.Size() == 0) // No files - hide this
			pItem->m_bIsFolder = true;
		else if (items.Size() == 1 && items[0]->m_idepth == 0x30)
		{
			// One STORED file - collapse it down
			*pItem = *items[0];
		}
		else
		{
			// Compressed or more than one file -> create a rar dir
			pItem->SetPath(strUrl);
//			return new CRarDirectory; // TODO
		}
		return NULL;
	}
/*	
	if (strExtension.Equals(".xsp")) // TODO
	{
		// XBMC Smart playlist - just XML renamed to XSP
		// read the name of the playlist in
		CSmartPlaylist playlist;
		if (playlist.OpenAndReadName(strPath))
		{
			pItem->SetLabel(playlist.GetName());
			pItem->SetLabelPreformated(true);
		}
		IFileDirectory* pDir=new CSmartPlaylistDirectory;
		return pDir; // Treat as directory
	}
*/
/*	
	if (g_advancedSettings.m_playlistAsFolders && CPlayListFactory::IsPlaylist(strPath)) // TODO
	{
		// Playlist file
		// currently we only return the directory if it contains
		// more than one file.  Reason is that .pls and .m3u may be used
		// for links to http streams etc. 
		IFileDirectory *pDir = new CPlaylistFileDirectory();
		CFileItemList items;
		
		if (pDir->GetDirectory(strPath, items))
		{
			if (items.Size() > 1)
				return pDir;
		}
		delete pDir;
		return NULL;
	}*/
	return NULL;
}


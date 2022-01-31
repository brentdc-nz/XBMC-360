#include "FileItem.h"
#include "utils\log.h"
#include "utils\Util.h"
#include "Settings.h"

CFileItem::CFileItem(void)
{
	Reset();
	m_strPath = "";
}

CFileItem::CFileItem(const CStdString& strLabel)
{
	Reset();
	SetLabel(strLabel);
}

CFileItem::CFileItem(const CStdString& strPath, bool bIsFolder)
{
	Reset();
	m_strPath = strPath;
	m_bIsFolder = bIsFolder;
	m_bIsParentFolder = false;
}

CFileItem::CFileItem(const CMediaSource& share) //TODO
{
	Reset();
	m_bIsFolder = true;
	m_bIsShareOrDrive = true;
	m_strPath = share.strPath;
	m_strLabel = share.strName;
	m_bIsParentFolder = false;

//	if(share.strStatus.size())
//		m_strLabel.Format("%s (%s)", share.strName.c_str(), share.strStatus.c_str());

	m_iDriveType = share.m_iDriveType;
//	m_strThumbnailImage = share.m_strThumbnailImage;
}

CFileItem::~CFileItem()
{
}

void CFileItem::Reset()
{
	m_strLabel.Empty();
	m_strPath.Empty();
	m_bIsFolder = false;
	m_bIsParentFolder = false;
}

bool CFileItem::IsXEX() const
{
	return CUtil::GetExtension(m_strPath).Equals(".xex", false);
}

bool CFileItem::IsVideo() const
{
	CStdString extension;
	CUtil::GetExtension(m_strPath, extension);

	if(extension.IsEmpty())
		return false;

	extension.ToLower();

	if(g_settings.GetVideoExtensions().Find(extension) != -1)
		return true;

	return false;
}

bool CFileItem::IsAudio() const
{
//	if(IsCDDA()) return true; // TODO

	CStdString extension;
	CUtil::GetExtension(m_strPath, extension);

	if(extension.IsEmpty())
		return false;

	extension.ToLower();

	if(g_settings.GetAudioExtensions().Find(extension) != -1)
		return true;

	return false;
}

bool CFileItem::IsPicture() const
{
	CStdString extension;
	CUtil::GetExtension(m_strPath, extension);

	if(extension.IsEmpty())
		return false;

	extension.ToLower();

	if(g_settings.GetPictureExtensions().Find(extension) != -1)
		return true;

	return false;
}

bool CFileItem::IsParentFolder() const
{
	return m_bIsParentFolder;
}

bool CFileItem::IsFileFolder() const // More folder types to come
{
	return m_bIsFolder;
}

bool CFileItem::IsVirtualDirectoryRoot() const
{
	return (m_bIsFolder && m_strPath.IsEmpty());
}

const CStdString &CFileItem::GetPath() const
{
	return m_strPath;
}

void CFileItem::SetPath(CStdString strPath)
{
	m_strPath = strPath;
}

void CFileItem::SetLabel(const CStdString &strLabel)
{
	m_strLabel = strLabel;
	if(strLabel == "..")
	{
		m_bIsParentFolder = true;
		m_bIsFolder = true;
//		SetLabelPreformated(true);
	}
}

void CFileItem::FillInDefaultIcon()
{
	CLog::Log(LOGINFO, "FillInDefaultIcon(%s)", GetLabel().c_str());
	// Find the default icon for a file or folder item
	// for files this can be the (depending on the file type)
	// default picture for photo's
	// default picture for songs
	// default picture for videos
	// default picture for shortcuts
	// default picture for playlists
	// or the icon embedded in an .xex

	// for folders
	// for .. folders the default picture for parent folder
	// for other folders the defaultFolder.png

	CStdString strThumb;
	CStdString strExtension;
	if(GetIconImage() == "")
	{
		if(!m_bIsFolder)
		{
			if(IsXEX())
			{
				// XEX
				SetIconImage("defaultProgram.png");
			}
			else if(IsVideo())
			{
				// Video
				SetIconImage("defaultVideo.png");
			}
			else if(IsAudio())
			{
				// Audio
				SetIconImage("defaultAudio.png");
			}
			else if(IsPicture())
			{
				// Picture
				SetIconImage("defaultPicture.png");
			}
			else
			{
				// Default icon for unknown file type
				SetIconImage("DefaultFile.png");
			}
		}
		else
		{
			if(IsParentFolder())
				SetIconImage("defaultFolderBack.png");
			else
				SetIconImage("defaultFolder.png");
		}
	}
}

const CStdString& CFileItem::GetContentType() const
{
	if(m_contenttype.IsEmpty())
	{
		// Discard const qualifyier
		CStdString& m_ref = (CStdString&)m_contenttype;

		// TODO!

		// If it's still empty set to an unknown type
		if(m_ref.IsEmpty())
			m_ref = "application/octet-stream";
	}

	return m_contenttype;
}

/////////////////////////////////////////////////////////////////////////////////
//
// CFileItemList
//
//////////////////////////////////////////////////////////////////////////////////

CFileItemList::CFileItemList()
{
	m_fastLookup = false;
	m_bIsFolder = true;
}

CFileItemList::~CFileItemList()
{
	Clear();
}

CFileItem* CFileItemList::operator[] (int iItem)
{
	return Get(iItem);
}

const CFileItem* CFileItemList::operator[] (int iItem) const
{
	return Get(iItem);
}

CFileItem* CFileItemList::Get(int iItem)
{
	return m_items[iItem];
}

const CFileItem* CFileItemList::Get(int iItem) const
{
	return m_items[iItem];
}

void CFileItemList::Remove(int iItem)
{
	if(iItem >= 0 && iItem < (int)Size())
	{
		CFileItem* pItem = *(m_items.begin() + iItem);
		if(m_fastLookup)
			m_map.erase(pItem->GetPath());

		delete pItem;
		m_items.erase(m_items.begin() + iItem);
	}
}

void CFileItemList::Add(CFileItem* pItem)
{
	m_items.push_back(pItem);
}

int CFileItemList::Size() const
{
	return (int)m_items.size();
}

bool CFileItemList::IsEmpty() const
{
	return (m_items.size() <= 0);
}

void CFileItemList::Append(const CFileItemList& itemlist)
{
	for(int i = 0; i < itemlist.Size(); ++i)
	{
		const CFileItem* pItem = itemlist[i];
		CFileItem* pNewItem = new CFileItem(*pItem);
		Add(pNewItem);
	}
}

void CFileItemList::FillInDefaultIcons()
{
	for(int i = 0; i < (int)m_items.size(); ++i)
	{
		CFileItem* pItem = m_items[i];
		pItem->FillInDefaultIcon();
	}
}

void CFileItemList::Clear()
{
	if(m_items.size())
	{
		IVECFILEITEMS i;
		i = m_items.begin();

		while(i != m_items.end())
		{
			CFileItem* pItem = *i;
			delete pItem;
			i = m_items.erase(i);
		}
		m_map.clear();
	}
}

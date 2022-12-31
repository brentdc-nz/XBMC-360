#include "FileItem.h"
#include "utils\Log.h"
#include "utils\Util.h"
#include "utils\URIUtils.h"
#include "Settings.h"

CFileItem::CFileItem(void)
{
	Reset();
}

CFileItem::CFileItem(const CFileItem& item): CGUIListItem()
{
	*this = item;
}

CFileItem::CFileItem(const CGUIListItem& item)
{
	Reset();
	// Not particularly pretty, but it gets around the issue of Reset() defaulting
	// parameters in the CGUIListItem base class
	*((CGUIListItem *)this) = item;
}

CFileItem::CFileItem(const CStdString& strLabel)
    : CGUIListItem()
{
	Reset();
	SetLabel(strLabel);
}

CFileItem::CFileItem(const CStdString& strPath, bool bIsFolder)
{
	Reset();
	m_strPath = strPath;
	m_bIsFolder = bIsFolder;
}

CFileItem::CFileItem(const CMediaSource& share)
{
	Reset();

	m_bIsFolder = true;
	m_bIsShareOrDrive = true;
	m_strPath = share.strPath;
	URIUtils::AddSlashAtEnd(m_strPath);
	CStdString label = share.strName;

	if (!share.strStatus.IsEmpty())
		label.Format("%s (%s)", share.strName.c_str(), share.strStatus.c_str());

	SetLabel(label);
	m_iDriveType = share.m_iDriveType;
	m_strThumbnailImage = share.m_strThumbnailImage;
	SetLabelPreformated(true);
}

CFileItem::~CFileItem(void)
{
}

const CFileItem& CFileItem::operator=(const CFileItem& item)
{
	if (this == &item) return * this;

	CGUIListItem::operator=(item);

	m_bLabelPreformated=item.m_bLabelPreformated;

	FreeMemory();

	m_strPath = item.GetPath();
	m_bIsParentFolder = item.m_bIsParentFolder;
	m_bIsShareOrDrive = item.m_bIsShareOrDrive;
	m_dateTime = item.m_dateTime;
	m_dwSize = item.m_dwSize;

	m_lStartOffset = item.m_lStartOffset;
	m_lEndOffset = item.m_lEndOffset;
	m_iprogramCount = item.m_iprogramCount;
	m_idepth = item.m_idepth;
	m_mimetype = item.m_mimetype;

	return *this;
}

void CFileItem::Reset() // TODO
{
	m_strLabel2.Empty();
	SetLabel("");
	m_bLabelPreformated = false;
	FreeIcons();
	m_overlayIcon = ICON_OVERLAY_NONE;
	m_bSelected = false;
//	m_strDVDLabel.Empty();
//	m_strTitle.Empty();
	m_strPath.Empty();
	m_dwSize = 0;
	m_bIsFolder = false;
	m_bIsParentFolder = false;
	m_bIsShareOrDrive = false;
	m_dateTime.Reset();
	m_iDriveType = CMediaSource::SOURCE_TYPE_UNKNOWN;
	m_lStartOffset = 0;
	m_lEndOffset = 0;
	m_iprogramCount = 0;
	m_idepth = 1;
//	m_iLockMode = LOCK_MODE_EVERYONE;
//	m_strLockCode = "";
//	m_iBadPwdCount = 0;
//	m_iHasLock = 0;
//	m_bCanQueue=true;
	m_mimetype = "";
//	delete m_musicInfoTag;
//	m_musicInfoTag = NULL;
//	delete m_videoInfoTag;
//	m_videoInfoTag = NULL;
//	delete m_pictureInfoTag;
//	m_pictureInfoTag = NULL;
//	m_extrainfo.Empty();
//	m_specialSort = SORT_NORMALLY;
	SetInvalid();
}

bool CFileItem::IsVideo() const
{
	// Check preset mime type
	if(m_mimetype.Left(6).Equals("video/"))
		return true;

/* // TODO
	if (HasVideoInfoTag()) return true;
	if (HasMusicInfoTag()) return false;
	if (HasPictureInfoTag()) return false;

	if (IsHDHomeRun() || IsTuxBox() || URIUtils::IsDVD(m_strPath) || IsSlingbox())
		return true;
*/
	CStdString extension;
	if( m_mimetype.Left(12).Equals("application/") )
	{
		// Check for some standard types
		extension = m_mimetype.Mid(12);
		if(extension.Equals("ogg")
			|| extension.Equals("mp4")
			|| extension.Equals("mxf"))
		return true;
	}

	URIUtils::GetExtension(m_strPath, extension);

	if (extension.IsEmpty())
		return false;

	extension.ToLower();

	if (g_settings.GetVideoExtensions().Find(extension) != -1)
		return true;

	return false;
}

bool CFileItem::IsAudio() const
{
	// Check preset mime type
	if( m_mimetype.Left(6).Equals("audio/") )
		return true;

/* // TODO
	if (HasMusicInfoTag()) return true;
	if (HasVideoInfoTag()) return false;
	if (HasPictureInfoTag()) return false;
	if (IsCDDA()) return true;
	if (!m_bIsFolder && IsShoutCast()) return true;
	if (!m_bIsFolder && IsLastFM()) return true;
*/
	CStdString extension;
	
	if( m_mimetype.Left(12).Equals("application/") )
	{
		// Check for some standard types
		extension = m_mimetype.Mid(12);
		if( extension.Equals("ogg")
			|| extension.Equals("mp4")
			|| extension.Equals("mxf") )
		return true;
	}

	URIUtils::GetExtension(m_strPath, extension);

	if (extension.IsEmpty())
		return false;

	extension.ToLower();

	if (g_settings.GetAudioExtensions().Find(extension) != -1)
		return true;

	return false;
}

bool CFileItem::IsPicture() const
{
	if( m_mimetype.Left(6).Equals("image/") )
		return true;

/* // TODO
	if (HasPictureInfoTag()) return true;
	if (HasMusicInfoTag()) return false;
	if (HasVideoInfoTag()) return false;
*/
	return CUtil::IsPicture(m_strPath);
}

bool CFileItem::IsXEX() const
{
	return URIUtils::GetExtension(m_strPath).Equals(".xex", false);
}

bool CFileItem::IsParentFolder() const
{
	return m_bIsParentFolder;
}

bool CFileItem::IsVirtualDirectoryRoot() const
{
	return (m_bIsFolder && m_strPath.IsEmpty());
}

bool CFileItem::IsRemovable() const
{
	return false; //TODO!!!
}

bool CFileItem::IsFileFolder() const
{
	return false; //TODO!!!
}

bool CFileItem::IsRemote() const
{
	return URIUtils::IsRemote(m_strPath);
}

void CFileItem::FillInDefaultIcon()
{
	CLog::Log(LOGINFO, "FillInDefaultIcon(%s)", GetLabel().c_str());
	// Find the default icon for a file or folder item
	// for files this can be the (depending on the file type)
	//   default picture for photo's
	//   default picture for songs
	//   default picture for videos
	//   default picture for shortcuts
	//   default picture for playlists
	//   or the icon embedded in an .xbe
	//
	// For folders
	//   for .. folders the default picture for parent folder
	//   for other folders the defaultFolder.png

	if (GetIconImage().IsEmpty())
	{
		if (!m_bIsFolder)
		{
			/* To reduce the average runtime of this code, this list should
			* be ordered with most frequently seen types first. Also bear
			* in mind the complexity of the code behind the check in the
			* case of IsWhatater() returns false.
			*/
			if (IsAudio())
			{
				// Audio
				SetIconImage("DefaultAudio.png");
			}
			else if (IsVideo())
			{
				// Video
				SetIconImage("DefaultVideo.png");
			}
			else if (IsPicture())
			{
				// Picture
				SetIconImage("DefaultPicture.png");
			}
/*			if (IsPlayList())
			{
				SetIconImage("DefaultPlaylist.png");
			}
*/			else if (IsXEX())
			{
				// xex
				SetIconImage("DefaultProgram.png");
			}
/*			else if (IsShortCut() && !IsLabelPreformated())
			{
				// Shortcut
				CStdString strFName = URIUtils::GetFileName(m_strPath);
				int iPos = strFName.ReverseFind(".");
				CStdString strDescription = strFName.Left(iPos);
				SetLabel(strDescription);
				SetIconImage("DefaultShortcut.png");
			}
			else if (IsPythonScript())
			{
				SetIconImage("DefaultScript.png");
			}
*/			else
			{
				// Default icon for unknown file type
				SetIconImage("DefaultFile.png");
			}
		}
		else
		{
/*			if (IsPlayList()) //TODO
			{
				SetIconImage("DefaultPlaylist.png");
			}
			else*/ if (IsParentFolder())
			{
				SetIconImage("DefaultFolderBack.png");
			}
			else
			{
				SetIconImage("DefaultFolder.png");
			}
		}
	}
	
/* // TODO
	// Set the icon overlays (if applicable)
	if (!HasOverlay())
	{
		if (URIUtils::IsInRAR(m_strPath))
			SetOverlayImage(CGUIListItem::ICON_OVERLAY_RAR);
		else if (URIUtils::IsInZIP(m_strPath))
			SetOverlayImage(CGUIListItem::ICON_OVERLAY_ZIP);
	}*/
}

void CFileItem::SetLabel(const CStdString &strLabel)
{
	if (strLabel=="..")
	{
		m_bIsParentFolder=true;
		m_bIsFolder=true;
//		m_specialSort = SORT_ON_TOP; // TODO
		SetLabelPreformated(true);
	}
	CGUIListItem::SetLabel(strLabel);
}

void CFileItem::RemoveExtension()
{
	if (m_bIsFolder)
		return;

	CStdString strLabel = GetLabel();
	URIUtils::RemoveExtension(strLabel);
	SetLabel(strLabel);
}


CURL CFileItem::GetAsUrl() const
{
	return CURL(m_strPath);
}

/////////////////////////////////////////////////////////////////////////////////
/////
///// CFileItemList
/////
//////////////////////////////////////////////////////////////////////////////////

CFileItemList::CFileItemList()
{
	m_fastLookup = false;
	m_bIsFolder = true;
	m_cacheToDisc = CACHE_IF_SLOW;
	m_sortMethod = SORT_METHOD_NONE;
	m_sortOrder = SORT_ORDER_NONE;
	m_replaceListing = false;
}

CFileItemList::CFileItemList(const CStdString& strPath) : CFileItem(strPath, true)
{
	m_fastLookup = false;
	m_cacheToDisc=CACHE_IF_SLOW;
	m_sortMethod=SORT_METHOD_NONE;
	m_sortOrder=SORT_ORDER_NONE;
	m_replaceListing = false;
}

CFileItemList::~CFileItemList()
{
	Clear();
}

CFileItemPtr CFileItemList::operator[] (int iItem)
{
	return Get(iItem);
}

const CFileItemPtr CFileItemList::operator[] (int iItem) const
{
	return Get(iItem);
}

CFileItemPtr CFileItemList::operator[] (const CStdString& strPath)
{
	return Get(strPath);
}

const CFileItemPtr CFileItemList::operator[] (const CStdString& strPath) const
{
	return Get(strPath);
}

void CFileItemList::Add(const CFileItemPtr &pItem)
{
	CSingleLock lock(m_lock);

	m_items.push_back(pItem);

	if (m_fastLookup)
	{
		CStdString path(pItem->GetPath());
		path.ToLower();
		m_map.insert(MAPFILEITEMSPAIR(path, pItem));
	}
}

void CFileItemList::AddFront(const CFileItemPtr &pItem, int itemPosition)
{
	CSingleLock lock(m_lock);

	if (itemPosition >= 0)
	{
		m_items.insert(m_items.begin()+itemPosition, pItem);
	}
	else
	{
		m_items.insert(m_items.begin()+(m_items.size()+itemPosition), pItem);
	}
	
	if (m_fastLookup)
	{
		CStdString path(pItem->GetPath());
		path.ToLower();
		m_map.insert(MAPFILEITEMSPAIR(path, pItem));
	}
}

void CFileItemList::Remove(int iItem)
{
	CSingleLock lock(m_lock);

	if (iItem >= 0 && iItem < (int)Size())
	{
		CFileItemPtr pItem = *(m_items.begin() + iItem);
		if (m_fastLookup)
		{
			CStdString path(pItem->GetPath());
			path.ToLower();
			m_map.erase(path);
		}
		m_items.erase(m_items.begin() + iItem);
	}
}

CFileItemPtr CFileItemList::Get(int iItem)
{
	CSingleLock lock(m_lock);

	if (iItem > -1 && iItem < (int)m_items.size())
		return m_items[iItem];

	return CFileItemPtr();
}

const CFileItemPtr CFileItemList::Get(int iItem) const
{
	CSingleLock lock(m_lock);

	if (iItem > -1 && iItem < (int)m_items.size())
		return m_items[iItem];

	return CFileItemPtr();
}

CFileItemPtr CFileItemList::Get(const CStdString& strPath)
{
	CSingleLock lock(m_lock);

	CStdString pathToCheck(strPath);
	pathToCheck.ToLower();
	
	if (m_fastLookup)
	{
		IMAPFILEITEMS it = m_map.find(pathToCheck);
		if (it != m_map.end())
			return it->second;

		return CFileItemPtr();
	}
	
	// Slow method...
	for (unsigned int i = 0; i < m_items.size(); i++)
	{
		CFileItemPtr pItem = m_items[i];
		if (pItem->GetPath().Equals(pathToCheck))
			return pItem;
	}

	return CFileItemPtr();
}

const CFileItemPtr CFileItemList::Get(const CStdString& strPath) const
{
	CSingleLock lock(m_lock);

	CStdString pathToCheck(strPath);
	pathToCheck.ToLower();
	
	if (m_fastLookup)
	{
		map<CStdString, CFileItemPtr>::const_iterator it=m_map.find(pathToCheck);
		if (it != m_map.end())
			return it->second;

		return CFileItemPtr();
	}
	
	// Slow method...
	for (unsigned int i = 0; i < m_items.size(); i++)
	{
		CFileItemPtr pItem = m_items[i];
		if (pItem->GetPath().Equals(pathToCheck))
			return pItem;
	}

	return CFileItemPtr();
}

void CFileItemList::Clear()
{
	CSingleLock lock(m_lock);

	ClearItems();

	m_replaceListing = false;
}

void CFileItemList::ClearItems()
{
	CSingleLock lock(m_lock);
	
	// Make sure we free the memory of the items (these are GUIControls which may have allocated resources)
	FreeMemory();
	
	for (unsigned int i = 0; i < m_items.size(); i++)
	{
		CFileItemPtr item = m_items[i];
		item->FreeMemory();
	}

	m_items.clear();
	m_map.clear();
}

int CFileItemList::Size() const
{
	CSingleLock lock(m_lock);
	return (int)m_items.size();
}

bool CFileItemList::IsEmpty() const
{
	CSingleLock lock(m_lock);
	return (m_items.size() <= 0);
}

bool CFileItemList::Copy(const CFileItemList& items)
{
	// Assign all CFileItem parts
	*(CFileItem*)this = *(CFileItem*)&items;

	// Assign the rest of the CFileItemList properties
	m_replaceListing = items.m_replaceListing;
	m_content        = items.m_content;
	m_mapProperties  = items.m_mapProperties;
	m_cacheToDisc    = items.m_cacheToDisc;
	m_sortDetails    = items.m_sortDetails;
	m_sortMethod     = items.m_sortMethod;
	m_sortOrder      = items.m_sortOrder;

	// Make a copy of each item
	for (int i = 0; i < items.Size(); i++)
	{
		CFileItemPtr newItem(new CFileItem(*items[i]));
		Add(newItem);
	}

	return true;
}

void CFileItemList::Sort(SORT_METHOD sortMethod, SORT_ORDER sortOrder) // TODO
{
  // Already sorted?
  if (sortMethod==m_sortMethod && m_sortOrder==sortOrder)
    return;

  switch (sortMethod)
  {
  case SORT_METHOD_LABEL:
  case SORT_METHOD_LABEL_IGNORE_FOLDERS:
    FillSortFields(SSortFileItem::ByLabel);
    break;
  case SORT_METHOD_LABEL_IGNORE_THE:
    FillSortFields(SSortFileItem::ByLabelNoThe);
    break;
  case SORT_METHOD_DATE:
    FillSortFields(SSortFileItem::ByDate);
    break;
  case SORT_METHOD_SIZE:
    FillSortFields(SSortFileItem::BySize);
    break;
  case SORT_METHOD_BITRATE:
    FillSortFields(SSortFileItem::ByBitrate);
    break;      
  case SORT_METHOD_DRIVE_TYPE:
    FillSortFields(SSortFileItem::ByDriveType);
    break;
  case SORT_METHOD_TRACKNUM:
    FillSortFields(SSortFileItem::BySongTrackNum);
    break;
  case SORT_METHOD_EPISODE:
    FillSortFields(SSortFileItem::ByEpisodeNum);
    break;
  case SORT_METHOD_DURATION:
    FillSortFields(SSortFileItem::BySongDuration);
    break;
  case SORT_METHOD_TITLE_IGNORE_THE:
    FillSortFields(SSortFileItem::BySongTitleNoThe);
    break;
  case SORT_METHOD_TITLE:
    FillSortFields(SSortFileItem::BySongTitle);
    break;
  case SORT_METHOD_ARTIST:
    FillSortFields(SSortFileItem::BySongArtist);
    break;
  case SORT_METHOD_ARTIST_IGNORE_THE:
    FillSortFields(SSortFileItem::BySongArtistNoThe);
    break;
  case SORT_METHOD_ALBUM:
    FillSortFields(SSortFileItem::BySongAlbum);
    break;
  case SORT_METHOD_ALBUM_IGNORE_THE:
    FillSortFields(SSortFileItem::BySongAlbumNoThe);
    break;
  case SORT_METHOD_GENRE:
    FillSortFields(SSortFileItem::ByGenre);
    break;
  case SORT_METHOD_DATEADDED:
    FillSortFields(SSortFileItem::ByDateAdded);
    break;
  case SORT_METHOD_FILE:
    FillSortFields(SSortFileItem::ByFile);
    break;
  case SORT_METHOD_VIDEO_RATING:
    FillSortFields(SSortFileItem::ByMovieRating);
    break;
  case SORT_METHOD_VIDEO_TITLE:
    FillSortFields(SSortFileItem::ByMovieTitle);
    break;
  case SORT_METHOD_VIDEO_SORT_TITLE:
    FillSortFields(SSortFileItem::ByMovieSortTitle);
    break;
  case SORT_METHOD_VIDEO_SORT_TITLE_IGNORE_THE:
    FillSortFields(SSortFileItem::ByMovieSortTitleNoThe);
    break;
  case SORT_METHOD_YEAR:
    FillSortFields(SSortFileItem::ByYear);
    break;
  case SORT_METHOD_PRODUCTIONCODE:
    FillSortFields(SSortFileItem::ByProductionCode);
    break;
  case SORT_METHOD_PROGRAM_COUNT:
  case SORT_METHOD_PLAYLIST_ORDER:
    // TODO: Playlist order is hacked into program count variable (not nice, but ok until 2.0)
    FillSortFields(SSortFileItem::ByProgramCount);
    break;
  case SORT_METHOD_SONG_RATING:
    FillSortFields(SSortFileItem::BySongRating);
    break;
  case SORT_METHOD_MPAA_RATING:
    FillSortFields(SSortFileItem::ByMPAARating);
    break;
  case SORT_METHOD_VIDEO_RUNTIME:
    FillSortFields(SSortFileItem::ByMovieRuntime);
    break;
  case SORT_METHOD_STUDIO:
    FillSortFields(SSortFileItem::ByStudio);
    break;
  case SORT_METHOD_STUDIO_IGNORE_THE:
    FillSortFields(SSortFileItem::ByStudioNoThe);
    break;
  case SORT_METHOD_FULLPATH:
    FillSortFields(SSortFileItem::ByFullPath);
    break;
  case SORT_METHOD_LASTPLAYED:
    FillSortFields(SSortFileItem::ByLastPlayed);
    break;
  case SORT_METHOD_LISTENERS:
    FillSortFields(SSortFileItem::ByListeners);
    break;    
  default:
    break;
  }
/*  if (sortMethod == SORT_METHOD_FILE        ||
      sortMethod == SORT_METHOD_VIDEO_SORT_TITLE ||
      sortMethod == SORT_METHOD_VIDEO_SORT_TITLE_IGNORE_THE ||
      sortMethod == SORT_METHOD_LABEL_IGNORE_FOLDERS)
    Sort(sortOrder==SORT_ORDER_ASC ? SSortFileItem::IgnoreFoldersAscending : SSortFileItem::IgnoreFoldersDescending); //TODO
  else if (sortMethod != SORT_METHOD_NONE && sortMethod != SORT_METHOD_UNSORTED)
    Sort(sortOrder==SORT_ORDER_ASC ? SSortFileItem::Ascending : SSortFileItem::Descending);*/

  m_sortMethod=sortMethod;
  m_sortOrder=sortOrder;
}

void CFileItemList::Append(const CFileItemList& itemlist)
{
	CSingleLock lock(m_lock);

	for (int i = 0; i < itemlist.Size(); ++i)
		Add(itemlist[i]);
}

void CFileItemList::Assign(const CFileItemList& itemlist, bool append)
{
	CSingleLock lock(m_lock);

	if (!append)
		Clear();

	Append(itemlist);
	SetPath(itemlist.GetPath());
	m_sortDetails = itemlist.m_sortDetails;
	m_replaceListing = itemlist.m_replaceListing;
	m_content = itemlist.m_content;
	m_mapProperties = itemlist.m_mapProperties;
	m_cacheToDisc = itemlist.m_cacheToDisc;
}

void CFileItemList::FillInDefaultIcons()
{
	CSingleLock lock(m_lock);
	for (int i = 0; i < (int)m_items.size(); ++i)
	{
		CFileItemPtr pItem = m_items[i];
		pItem->FillInDefaultIcon();
	}
}

void CFileItemList::SetFastLookup(bool fastLookup)
{
	CSingleLock lock(m_lock);

	if (fastLookup && !m_fastLookup)
	{
		// Generate the map
		m_map.clear();
		
		for (unsigned int i=0; i < m_items.size(); i++)
		{
			CFileItemPtr pItem = m_items[i];
			CStdString path(pItem->GetPath());
			path.ToLower();
			m_map.insert(MAPFILEITEMSPAIR(path, pItem));
		}
	}
	
	if (!fastLookup && m_fastLookup)
		m_map.clear();
	
	m_fastLookup = fastLookup;
}

bool CFileItemList::Contains(const CStdString& fileName) const
{
	CSingleLock lock(m_lock);

	// Checks case insensitive
	CStdString checkPath(fileName);
	checkPath.ToLower();
	
	if (m_fastLookup)
		return m_map.find(checkPath) != m_map.end();
	
	// Slow method...
	for (unsigned int i = 0; i < m_items.size(); i++)
	{
		const CFileItemPtr pItem = m_items[i];

		if (pItem->GetPath().Equals(checkPath))
			return true;
	}
	return false;
}

void CFileItemList::ClearSortState()
{
	m_sortMethod = SORT_METHOD_NONE;
	m_sortOrder = SORT_ORDER_NONE;
}

bool CFileItemList::Save(int windowID) //TODO
{
	int iSize = Size();

	if (iSize <= 0)
		return false;
/*
	CLog::Log(LOGDEBUG,"Saving fileitems [%s]",GetPath().c_str());

	CFile file;
	if (file.OpenForWrite(GetDiscCacheFile(windowID), true)) // overwrite always
	{
		CArchive ar(&file, CArchive::store);
		ar << *this;
		CLog::Log(LOGDEBUG,"  -- items: %i, sort method: %i, ascending: %s",iSize,m_sortMethod, m_sortOrder ? "true" : "false");
		ar.Close();
		file.Close();
		return true;
	}*/

  return false;
}

bool CFileItemList::UpdateItem(const CFileItem *item)
{
	if (!item) return false;
	
	CFileItemPtr oldItem = Get(item->GetPath());
	
	if (oldItem)
		*oldItem = *item;
	
	return oldItem;
}

bool CFileItemList::Load(int windowID)
{
/*	CFile file;
	if (file.Open(GetDiscCacheFile(windowID)))
	{
		CLog::Log(LOGDEBUG,"Loading fileitems [%s]",GetPath().c_str());
		CArchive ar(&file, CArchive::load);
		ar >> *this;
		CLog::Log(LOGDEBUG,"  -- items: %i, directory: %s sort method: %i, ascending: %s",Size(),GetPath().c_str(), m_sortMethod, m_sortOrder ? "true" : "false");
		ar.Close();
		file.Close();
		return true;
	}
	*/
	return false; //TODO - No caching yet
}


void CFileItemList::RemoveDiscCache(int windowID) const
{
	// TODO - No Cache yet
}

bool CFileItemList::AlwaysCache() const
{
	return false; // TODO - No Cache yet
}

int CFileItemList::GetObjectCount() const
{
	CSingleLock lock(m_lock);

	int numObjects = (int)m_items.size();

	if (numObjects && m_items[0]->IsParentFolder())
		numObjects--;

	return numObjects;
}

void CFileItemList::FillSortFields(FILEITEMFILLFUNC func)
{
	CSingleLock lock(m_lock);
	std::for_each(m_items.begin(), m_items.end(), func);
}
#include "playlists\PlayList.h"
#include "utils\URIUtils.h"
#include <sstream>
#include "FileSystem\File.h"
#include "utils\log.h"
#include "utils\TimeUtils.h"
#include "PlayListFactory.h"

using namespace std;
using namespace XFILE;
using namespace PLAYLIST;

CPlayList::CPlayList(void)
{
	m_strPlayListName = "";
	m_iPlayableItems = -1;
	m_bShuffled = false;
	m_bWasPlayed = false;
}

CPlayList::~CPlayList(void)
{
	Clear();
}

void CPlayList::Add(const CFileItemPtr &item, int iPosition, int iOrder)
{
	int iOldSize = size();

	if (iPosition < 0 || iPosition >= iOldSize)
		iPosition = iOldSize;
	
	if (iOrder < 0 || iOrder >= iOldSize)
		item->m_iprogramCount = iOldSize;
	else
		item->m_iprogramCount = iOrder;

	// videodb files are not supported by the filesystem as yet
//	if (item->IsVideoDb())
//		item->SetPath(item->GetVideoInfoTag()->m_strFileNameAndPath); // TODO

	// Increment the playable counter
	item->ClearProperty("unplayable");

	if (m_iPlayableItems < 0)
		m_iPlayableItems = 1;
	else
		m_iPlayableItems++;

	//CLog::Log(LOGDEBUG,"%s item:(%02i/%02i)[%s]", __FUNCTION__, iPosition, item->m_iprogramCount, item->GetPath().c_str());
	if (iPosition == iOldSize)
		m_vecItems.push_back(item);
	else
	{
		ivecItems it = m_vecItems.begin() + iPosition;
		m_vecItems.insert(it, 1, item);
		
		// Correct any duplicate order values
		if (iOrder < iOldSize)
			IncrementOrder(iPosition + 1, iOrder);
	}
}

bool CPlayList::Load(const CStdString& strFileName)
{
	Clear();
	URIUtils::GetDirectory(strFileName, m_strBasePath);

	// TODO : BRENT
	return true;
/*
	CFileStream file;
	if (!file.Open(strFileName))
		return false;

	if (file.GetLength() > 1024*1024)
	{
		CLog::Log(LOGWARNING, "%s - File is larger than 1 MB, most likely not a playlist", __FUNCTION__);
		return false;
	}

	return LoadData(file);*/
}

bool CPlayList::LoadData(istream &stream)
{
	// Try to read as a string
	std::stringstream ss;
	ss << stream.rdbuf();
	CStdString data = ss.str().c_str();
	return LoadData(data);
}

bool CPlayList::LoadData(const CStdString& strData)
{
	return false;
}

void CPlayList::Add(const CFileItemPtr &item)
{
	Add(item, -1, -1);
}

void CPlayList::Add(CPlayList& playlist)
{
	for (int i = 0; i < (int)playlist.size(); i++)
		Add(playlist[i], -1, -1);
}

void CPlayList::Add(CFileItemList& items)
{
	for (int i = 0; i < (int)items.Size(); i++)
		Add(items[i]);
}

void CPlayList::Insert(CPlayList& playlist, int iPosition /* = -1 */)
{
	// Out of bounds so just add to the end
	int iSize = size();
	
	if (iPosition < 0 || iPosition >= iSize)
	{
		Add(playlist);
		return;
	}

	for (int i = 0; i < (int)playlist.size(); i++)
	{
		int iPos = iPosition + i;
		Add(playlist[i], iPos, iPos);
	}
}

void CPlayList::Insert(CFileItemList& items, int iPosition /* = -1 */)
{
	// Out of bounds so just add to the end
	int iSize = size();
	
	if (iPosition < 0 || iPosition >= iSize)
	{
		Add(items);
		return;
	}

	for (int i = 0; i < (int)items.Size(); i++)
	{
		Add(items[i], iPosition + i, iPosition + i);
	}
}

int CPlayList::size() const
{
	return (int)m_vecItems.size();
}

void CPlayList::DecrementOrder(int iOrder)
{
	if (iOrder < 0) return;

	// It was the last item so do nothing
	if (iOrder == size()) return;

	// Fix all items with an order greater than the removed iOrder
	ivecItems it;
	it = m_vecItems.begin();
	
	while (it != m_vecItems.end())
	{
		CFileItemPtr item = *it;

		if (item->m_iprogramCount > iOrder)
		{
			//CLog::Log(LOGDEBUG,"%s fixing item at order %i", __FUNCTION__, item->m_iprogramCount);
			item->m_iprogramCount--;
		}
		++it;
	}
}

void CPlayList::IncrementOrder(int iPosition, int iOrder)
{
	if (iOrder < 0) return;

	// Fix all items with an order equal or greater to the added iOrder at iPos
	ivecItems it;
	it = m_vecItems.begin() + iPosition;
	
	while (it != m_vecItems.end())
	{
		CFileItemPtr item = *it;
		if (item->m_iprogramCount >= iOrder)
		{
			//CLog::Log(LOGDEBUG,"%s fixing item at order %i", __FUNCTION__, item->m_iprogramCount);
			item->m_iprogramCount++;
		}
		++it;
	}
}

const CFileItemPtr CPlayList::operator[] (int iItem) const
{
	if (iItem < 0 || iItem >= size())
	{
		CLog::Log(LOGERROR, "Error trying to retrieve an item that's out of range");
		return CFileItemPtr();
	}
	return m_vecItems[iItem];
}

CFileItemPtr CPlayList::operator[] (int iItem)
{
	if (iItem < 0 || iItem >= size())
	{
		CLog::Log(LOGERROR, "Error trying to retrieve an item that's out of range");
		return CFileItemPtr();
	}
	return m_vecItems[iItem];
}

// Remove item from playlist by position
void CPlayList::Remove(int position)
{
	int iOrder = -1;
	
	if (position >= 0 && position < (int)m_vecItems.size())
	{
		iOrder = m_vecItems[position]->m_iprogramCount;
		m_vecItems.erase(m_vecItems.begin() + position);
	}

	DecrementOrder(iOrder);
}

bool CPlayList::Expand(int position)
{
	CFileItemPtr item = m_vecItems[position];
	auto_ptr<CPlayList> playlist (CPlayListFactory::Create(*item.get()));
	
	if ( NULL == playlist.get())
		return false;

	if(!playlist->Load(item->GetPath()))
		return false;

	// Remove any item that points back to itself
	for(int i = 0;i<playlist->size();i++)
	{
		if( (*playlist)[i]->GetPath().Equals( item->GetPath() ) )
		{
			playlist->Remove(i);
			i--;
		}
	}

	if(playlist->size() <= 0)
		return false;

	Remove(position);
	Insert(*playlist, position);

	return true;
}

void CPlayList::Clear()
{
	m_vecItems.erase(m_vecItems.begin(), m_vecItems.end());
	m_strPlayListName = "";
	m_iPlayableItems = -1;
	m_bWasPlayed = false;
}

void CPlayList::Shuffle(int iPosition)
{
	if (size() == 0)
		// Nothing to shuffle, just set the flag for later
		m_bShuffled = true;
	else
	{
		if (iPosition >= size())
			return;
		
		if (iPosition < 0)
			iPosition = 0;
		
		CLog::Log(LOGDEBUG,"%s shuffling at pos:%i", __FUNCTION__, iPosition);

		ivecItems it = m_vecItems.begin() + iPosition;
		random_shuffle(it, m_vecItems.end());

		// The list is now shuffled!
		m_bShuffled = true;
	}
}

struct SSortPlayListItem
{
	static bool PlaylistSort(const CFileItemPtr &left, const CFileItemPtr &right)
	{
		return (left->m_iprogramCount <= right->m_iprogramCount);
	}
};

void CPlayList::UnShuffle()
{
	sort(m_vecItems.begin(), m_vecItems.end(), SSortPlayListItem::PlaylistSort);
	// The list is now unshuffled!
	m_bShuffled = false;
}

void CPlayList::SetUnPlayable(int iItem)
{
	if (iItem < 0 || iItem >= size())
	{
		CLog::Log(LOGWARNING, "Attempt to set unplayable index %d", iItem);
		return;
	}

	CFileItemPtr item = m_vecItems[iItem];
	
	if (!item->GetPropertyBOOL("unplayable"))
	{
		item->SetProperty("unplayable", true);
		m_iPlayableItems--;
	}
}

void CPlayList::UpdateItem(const CFileItem *item)
{
	if (!item) return;

	for (ivecItems it = m_vecItems.begin(); it != m_vecItems.end(); ++it)
	{
		CFileItemPtr playlistItem = *it;
		
		if (playlistItem->GetPath() == item->GetPath())
		{
			*playlistItem = *item;
			break;
		}
	}
}
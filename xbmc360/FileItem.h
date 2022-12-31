#ifndef CFILEITEM_H
#define CFILEITEM_H

#include "MediaSource.h"
#include "guilib\GUIListItem.h"
#include "utils\SingleLock.h"
#include "XBDateTime.h"
#include "SortFileItem.h"
#include "URL.h"

#include <map>
#include <vector>

class CFileItem : public CGUIListItem
{
public:
	CFileItem(void);
	CFileItem(const CFileItem& item);
	CFileItem(const CGUIListItem& item);
	CFileItem(const CStdString& strLabel);
	CFileItem(const CStdString& strPath, bool bIsFolder);
	CFileItem(const CMediaSource& share);
	virtual ~CFileItem(void);

	virtual CGUIListItem *Clone() const { return new CFileItem(*this); };

	const CStdString &GetPath() const { return m_strPath; };
	void SetPath(const CStdString &path) { m_strPath = path; };
	void Reset();
	const CFileItem& operator=(const CFileItem& item);
	virtual bool IsFileItem() const { return true; };

	bool IsVideo() const;
	bool IsAudio() const;
	bool IsPicture() const;
	bool IsXEX() const;
	bool IsParentFolder() const;
	bool IsVirtualDirectoryRoot() const;
	bool IsRemovable() const;
	bool IsFileFolder() const;
	bool IsInternetStream() const { return false; } ; // TODO
	bool IsRemote() const;

	virtual void SetLabel(const CStdString &strLabel);
	void FillInDefaultIcon();
	void RemoveExtension();
	void SetLabelPreformated(bool bYesNo) { m_bLabelPreformated = bYesNo; }
	bool IsLabelPreformated() const { return m_bLabelPreformated; }
	CURL GetAsUrl() const;

public:
	bool m_bIsShareOrDrive; // Is this a root share/drive
	int m_iDriveType; // If \e m_bIsShareOrDrive is \e true, use to get the share type. Types see: CMediaSource::m_iDriveType
	CDateTime m_dateTime; // File creation date & time
	__int64 m_dwSize; // File size (0 for folders)

	int m_iprogramCount;
	int m_idepth;
	int m_lStartOffset;
	int m_lEndOffset;

//private: // FIXME - Make private again!
	CStdString m_strPath; // Complete path to item
	bool m_bIsParentFolder;
	bool m_bLabelPreformated;

	CStdString m_mimetype;
};

/////////////////////////////////////////////////////////////////////////////////
/////
///// CFileItemList
/////
//////////////////////////////////////////////////////////////////////////////////

// A shared pointer to CFileItem
typedef std::shared_ptr<CFileItem> CFileItemPtr;

// A vector of pointer to CFileItem
typedef std::vector< CFileItemPtr > VECFILEITEMS;

// Iterator for VECFILEITEMS
typedef std::vector< CFileItemPtr >::iterator IVECFILEITEMS;

// A map of pointers to CFileItem
typedef std::map<CStdString, CFileItemPtr > MAPFILEITEMS;

// Iterator for MAPFILEITEMS
typedef std::map<CStdString, CFileItemPtr >::iterator IMAPFILEITEMS;

// Pair for MAPFILEITEMS
typedef std::pair<CStdString, CFileItemPtr > MAPFILEITEMSPAIR;

typedef bool (*FILEITEMLISTCOMPARISONFUNC) (const CFileItemPtr &pItem1, const CFileItemPtr &pItem2);
typedef void (*FILEITEMFILLFUNC) (CFileItemPtr &item);

// Represents a list of files
class CFileItemList	 : public CFileItem
{
public:
	enum CACHE_TYPE { CACHE_NEVER = 0, CACHE_IF_SLOW, CACHE_ALWAYS };

	CFileItemList();
	CFileItemList(const CStdString& strPath);
	virtual ~CFileItemList();

	CFileItemPtr operator[] (int iItem);
	const CFileItemPtr operator[] (int iItem) const;
	CFileItemPtr operator[] (const CStdString& strPath);
	const CFileItemPtr operator[] (const CStdString& strPath) const;

	void Add(const CFileItemPtr &pItem);
	void AddFront(const CFileItemPtr &pItem, int itemPosition);
	void Remove(int iItem);

	CFileItemPtr Get(int iItem);
	const CFileItemPtr Get(int iItem) const;
	CFileItemPtr Get(const CStdString& strPath);
	const CFileItemPtr Get(const CStdString& strPath) const;

	SORT_ORDER GetSortOrder() const { return m_sortOrder; }
	SORT_METHOD GetSortMethod() const { return m_sortMethod; }

	void Clear();
	void ClearItems();
	int Size() const;
	bool IsEmpty() const;
	bool Copy (const CFileItemList& item);
	void Sort(SORT_METHOD sortMethod, SORT_ORDER sortOrder);
	void Append(const CFileItemList& itemlist);
	void Assign(const CFileItemList& itemlist, bool append = false);

	bool GetReplaceListing() const { return m_replaceListing; };
	const std::vector<SORT_METHOD_DETAILS> &GetSortDetails() const { return m_sortDetails; };

	void FillInDefaultIcons();

	void SetFastLookup(bool fastLookup);
	bool Contains(const CStdString& fileName) const;

	void ClearSortState();
	bool CacheToDiscAlways() const { return m_cacheToDisc == CACHE_ALWAYS; }
	void SetCacheToDisc(CACHE_TYPE cacheToDisc) { m_cacheToDisc = cacheToDisc; }
	bool CacheToDiscIfSlow() const { return m_cacheToDisc == CACHE_IF_SLOW; }

	// windowID id of the window that's saving this list (defaults to 0)
	// return true if successful, false otherwise.
	bool Save(int windowID = 0);

	// Update an item in the item list
	// param item the new item, which we match based on path to an existing item in the list
	// return true if the item exists in the list (and was thus updated), false otherwise.
	bool UpdateItem(const CFileItem *item);

	// windowID id of the window that's loading this list (defaults to 0)
	// return true if we loaded from the cache, false otherwise.
	// sa Save,RemoveDiscCache
	bool Load(int windowID = 0);

	void FillInDefaultIcon();

	void RemoveDiscCache(int windowID = 0) const;

	int GetObjectCount() const;

	bool AlwaysCache() const;

private:
	void FillSortFields(FILEITEMFILLFUNC func);

	VECFILEITEMS m_items;
	MAPFILEITEMS m_map;

	bool m_fastLookup;
	SORT_METHOD m_sortMethod;
	SORT_ORDER m_sortOrder;
	CACHE_TYPE m_cacheToDisc;
	bool m_replaceListing;
	CStdString m_content;

	std::vector<SORT_METHOD_DETAILS> m_sortDetails;

	CCriticalSection m_lock;
};

#endif //CFILEITEM_H
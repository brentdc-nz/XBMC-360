#ifndef H_CFILEITEM
#define H_CFILEITEM

#include "MediaManager.h"
#include "guilib\GUIListItem.h"
#include <map>
#include <vector>

class CFileItem : public CGUIListItem
{
public:
	CFileItem(void);
	CFileItem(const CStdString& strLabel);
	CFileItem(const CStdString& strPath, bool bIsFolder);
	CFileItem(const CMediaSource& share);
	~CFileItem();

	bool IsXEX() const;
	bool IsVideo() const;
	bool IsAudio() const;
	bool IsPicture() const;
	bool IsParentFolder() const;
	bool IsFileFolder() const;
	bool IsVirtualDirectoryRoot() const;

	void Reset();
	const CStdString &GetPath() const;
	void SetPath(CStdString strPath);
	virtual void SetLabel(const CStdString &strLabel);
	void FillInDefaultIcon();

    // Returns the content type of this item if known, will lookup for http streams 
	const CStdString& GetContentType() const; 

	bool m_bIsShareOrDrive; // Is this a root share/drive
	CStdString m_strPath; //FIXME: Make private again

private:
//	CStdString m_strPath;
	CStdString m_contenttype;
	bool m_bIsParentFolder;
	int m_iDriveType;
};

typedef std::vector<CFileItem*> VECFILEITEMS;

typedef std::map<CStdString, CFileItem*> MAPFILEITEMS;

typedef std::pair<CStdString, CFileItem*> MAPFILEITEMSPAIR;

typedef std::vector<CFileItem*>::iterator IVECFILEITEMS;

class CFileItemList : public CFileItem
{
public:
	CFileItemList();
	CFileItemList(const CStdString& strPath);
	virtual ~CFileItemList();

	CFileItem* operator[] (int iItem);
	const CFileItem* operator[] (int iItem) const;

	void Clear();
	CFileItem* Get(int iItem);
	const CFileItem* Get(int iItem) const;
	void Remove(int iItem);
	void Add(CFileItem* pItem);
	int Size() const;
	bool IsEmpty() const;
	void Append(const CFileItemList& itemlist);
	void FillInDefaultIcons();

private:
	bool m_fastLookup;
	VECFILEITEMS m_items;
	MAPFILEITEMS m_map;
};

#endif //H_CFILEITEM
#pragma once

#include "FileItem.h"

namespace PLAYLIST
{
class CPlayList
{
public:
	CPlayList(void);
	virtual ~CPlayList(void);
	virtual bool Load(const CStdString& strFileName);
	virtual bool LoadData(std::istream &stream);
	virtual bool LoadData(const CStdString& strData);

	void Add(CPlayList& playlist);
	void Add(const CFileItemPtr &pItem);
	void Add(CFileItemList& items);

	// For Party Mode
	void Insert(CPlayList& playlist, int iPosition = -1);
	void Insert(CFileItemList& items, int iPosition = -1);

	int size() const;
	const CFileItemPtr operator[] (int iItem) const;
	CFileItemPtr operator[] (int iItem);
	void Remove(int position);
	bool Expand(int position); // Expands any playlist at position into this playlist
	void Clear();

	void SetPlayed(bool bPlayed) { m_bWasPlayed = true; };
	bool WasPlayed() const { return m_bWasPlayed; };
 
	void Shuffle(int iPosition = 0);
	void UnShuffle();
	bool IsShuffled() const { return m_bShuffled; }

	void SetUnPlayable(int iItem);
	int GetPlayable() const { return m_iPlayableItems; };

	void UpdateItem(const CFileItem *item);

protected:
	CStdString m_strPlayListName;
	CStdString m_strBasePath;
	int m_iPlayableItems;
	bool m_bShuffled;
	bool m_bWasPlayed;
	std::vector <CFileItemPtr> m_vecItems;
	typedef std::vector <CFileItemPtr>::iterator ivecItems;

private:
	void Add(const CFileItemPtr& item, int iPosition, int iOrderOffset);
	void DecrementOrder(int iOrder);
	void IncrementOrder(int iPosition, int iOrder);
};
}

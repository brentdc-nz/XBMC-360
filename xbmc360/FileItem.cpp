#include "FileItem.h"

CFileItem::CFileItem(void)
{
	m_strPath = "";
}

CFileItem::CFileItem(const CStdString& strLabel)
{
	m_strPath = "";
	m_strLabel = strLabel;
}

CFileItem::~CFileItem()
{
}

/////////////////////////////////////////////////////////////////////////////////
/////
///// CFileItemList
/////
//////////////////////////////////////////////////////////////////////////////////

CFileItemList::CFileItemList()
{
}

CFileItemList::~CFileItemList()
{
	Clear();
}

CFileItem* CFileItemList::operator[] (int iItem)
{
  return Get(iItem);
}

CFileItem* CFileItemList::Get(int iItem)
{
	return m_items[iItem];
}

void CFileItemList::Add(CFileItem* pItem)
{
	m_items.push_back(pItem);
}

int CFileItemList::Size() const
{
	return (int)m_items.size();
}

void CFileItemList::Clear()
{
	if (m_items.size())
	{
		IVECFILEITEMS i;
		i = m_items.begin();
		while (i != m_items.end())
		{
			CFileItem* pItem = *i;
			delete pItem;
			i = m_items.erase(i);
		}
		m_map.clear();
	}
}

#ifndef H_CFILEITEM
#define H_CFILEITEM

#include "guilib\GUIListItem.h"
#include <map>
#include <vector>

class CFileItem : public CGUIListItem
{
public:
	CFileItem(void);
	CFileItem(const CStdString& strLabel);
	~CFileItem();

	void SetPath(CStdString strPath) { m_strPath = strPath; };

private:
	CStdString m_strPath;
};

/*!
  \brief A vector of pointer to CFileItem
  \sa CFileItem
  */
typedef std::vector<CFileItem*> VECFILEITEMS;

/*
  \brief A map of pointers to CFileItem
  \sa CFileItem
  */
typedef std::map<CStdString, CFileItem*> MAPFILEITEMS;

/*
  \brief Pair for MAPFILEITEMS
  \sa MAPFILEITEMS
  */
typedef std::pair<CStdString, CFileItem*> MAPFILEITEMSPAIR;

/*!
  \brief Iterator for VECFILEITEMS
  \sa CFileItemList
  */
typedef std::vector<CFileItem*>::iterator IVECFILEITEMS;

class CFileItemList : public CFileItem
{
public:
	CFileItemList();
	CFileItemList(const CStdString& strPath);
	virtual ~CFileItemList();

	CFileItem* operator[] (int iItem);

	void Clear();
	CFileItem* Get(int iItem);
	void Add(CFileItem* pItem);
	int Size() const;

private:
	VECFILEITEMS m_items;
	MAPFILEITEMS m_map;
};

#endif //H_CFILEITEM
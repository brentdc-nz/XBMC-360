#ifndef H_CTHUMBLOADER
#define H_CTHUMBLOADER

#include "BackgroundInfoLoader.h"
#include "guilib\GUIImage.h" // TEST - REMOVE ME

class CVideoThumbLoader : public CBackgroundInfoLoader
{
public:
	CVideoThumbLoader();
	virtual ~CVideoThumbLoader();
	virtual bool LoadItem(CFileItem* pItem);

private:
	CGUIImage* m_pThumb; // TEST - REMOVE ME
};

// TODO - Other thumb types

/*
class CProgramThumbLoader : public CBackgroundInfoLoader
{
public:
	CProgramThumbLoader();
	virtual ~CProgramThumbLoader();
	virtual bool LoadItem(CFileItem* pItem);
};

class CMusicThumbLoader : public CBackgroundInfoLoader
{
public:
	CMusicThumbLoader();
	virtual ~CMusicThumbLoader();
	virtual bool LoadItem(CFileItem* pItem);
};
*/

#endif //H_CTHUMBLOADER
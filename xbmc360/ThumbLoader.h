#ifndef H_CTHUMBLOADER
#define H_CTHUMBLOADER

#include "BackgroundInfoLoader.h"

class CVideoThumbLoader : public CBackgroundInfoLoader
{
public:
	CVideoThumbLoader();
	virtual ~CVideoThumbLoader();

	virtual bool LoadItem(CFileItem* pItem);

private:
};

class CProgramThumbLoader : public CBackgroundInfoLoader
{
public:
	CProgramThumbLoader();
	virtual ~CProgramThumbLoader();

	virtual bool LoadItem(CFileItem* pItem);

private:
};

class CMusicThumbLoader : public CBackgroundInfoLoader
{
public:
	CMusicThumbLoader();
	virtual ~CMusicThumbLoader();

	virtual bool LoadItem(CFileItem* pItem);

private:
};

class CPicturesThumbLoader : public CBackgroundInfoLoader
{
public:
	CPicturesThumbLoader();
	virtual ~CPicturesThumbLoader();

	virtual bool LoadItem(CFileItem* pItem);

private:
};

#endif //H_CTHUMBLOADER
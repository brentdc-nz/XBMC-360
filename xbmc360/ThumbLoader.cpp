#include "ThumbLoader.h"

CVideoThumbLoader::CVideoThumbLoader()
{
}

CVideoThumbLoader::~CVideoThumbLoader()
{
}

bool CVideoThumbLoader::LoadItem(CFileItem* pItem)
{
	return true;
}

//=================================================

CProgramThumbLoader::CProgramThumbLoader()
{
}

CProgramThumbLoader::~CProgramThumbLoader()
{
}

bool CProgramThumbLoader::LoadItem(CFileItem *pItem)
{
	return true;
}

//=================================================

CMusicThumbLoader::CMusicThumbLoader()
{
}

CMusicThumbLoader::~CMusicThumbLoader()
{
}

bool CMusicThumbLoader::LoadItem(CFileItem* pItem)
{
	return true;
}

//=================================================

CPicturesThumbLoader::CPicturesThumbLoader()
{
}

CPicturesThumbLoader::~CPicturesThumbLoader()
{
}

bool CPicturesThumbLoader::LoadItem(CFileItem* pItem)
{
	return true;
}
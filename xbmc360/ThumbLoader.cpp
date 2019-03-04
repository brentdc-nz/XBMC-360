#include "ThumbLoader.h"

CVideoThumbLoader::CVideoThumbLoader()
{
	m_pThumb = NULL;

	// TEST REMOVE ME
	CTextureInfo TInfo;
	TInfo.filename = "DefaultVideo.png";
	TInfo.useLarge = true;
	TInfo.orientation = 0;

	m_pThumb = new CGUIImage(0, 0, 0, 0, (float)90, (float)90, TInfo);
	m_pThumb->AllocResources();
	//TEST END
}

CVideoThumbLoader::~CVideoThumbLoader()
{
	// TEST REMOVE ME
	if(m_pThumb)
	{
		m_pThumb->FreeResources();
		delete m_pThumb;
	}
	m_pThumb = NULL;
	//TEST END
}

bool CVideoThumbLoader::LoadItem(CFileItem* pItem)
{
	// TODO

/*	if(pItem->m_bIsShareOrDrive)
		return true;

	if(!pItem->HasThumbnail())
		pItem->SetUserVideoThumb();
*/

	// TEST START - REMOVE BLOCK

	if(!pItem->GetThumbnail())
		pItem->SetThumbnail(m_pThumb);	

	if(pItem->GetThumbnail())
		CGUIImage *pImage = pItem->GetThumbnail();

	// TEST END

	return true;
}

// TODO - Other thumb types

/*
CProgramThumbLoader::CProgramThumbLoader()
{
}

CProgramThumbLoader::~CProgramThumbLoader()
{
}

bool CProgramThumbLoader::LoadItem(CFileItem *pItem)
{
	if(pItem->m_bIsShareOrDrive)
		return true;

	if(!pItem->HasThumbnail())
		pItem->SetUserProgramThumb();

	return true;
}
*/

CMusicThumbLoader::CMusicThumbLoader()
{
	// TEST REMOVE ME
	CTextureInfo TInfo;
	TInfo.filename = "DefaultAudio.png";
	TInfo.useLarge = true;
	TInfo.orientation = 0;

	m_pThumb = new CGUIImage(0, 0, 0, 0, (float)90, (float)90, TInfo);
	m_pThumb->AllocResources();
	//TEST END
}

CMusicThumbLoader::~CMusicThumbLoader()
{
	// TEST REMOVE ME
	if(m_pThumb)
	{
		m_pThumb->FreeResources();
		delete m_pThumb;
	}
	m_pThumb = NULL;
	//TEST END
}

bool CMusicThumbLoader::LoadItem(CFileItem* pItem)
{
/*	if (pItem->m_bIsShareOrDrive)
		return true;

	if (!pItem->HasThumbnail())
		pItem->SetUserMusicThumb();
*/
	// TEST START - REMOVE BLOCK

	if(!pItem->GetThumbnail())
		pItem->SetThumbnail(m_pThumb);	

	if(pItem->GetThumbnail())
		CGUIImage *pImage = pItem->GetThumbnail();

	// TEST END

	return true;
}

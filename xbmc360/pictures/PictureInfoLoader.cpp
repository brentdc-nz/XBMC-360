#include "pictures/PictureInfoLoader.h"
#include "pictures/PictureInfoTag.h"
#include "GUISettings.h"
#include "FileItem.h"

CPictureInfoLoader::CPictureInfoLoader()
	: CBackgroundInfoLoader(1) // Single worker: reads whole image files (EXIF) and
	                           // the SMB layer serializes on one lock anyway
{
	m_mapFileItems = new CFileItemList;
}

CPictureInfoLoader::~CPictureInfoLoader()
{
	StopThread();
	delete m_mapFileItems;
}

void CPictureInfoLoader::OnLoaderStart()
{
	// Load previously cached items from HD
	m_mapFileItems->SetPath(m_pVecItems->GetPath());
	m_mapFileItems->Load();
	m_mapFileItems->SetFastLookup(true);

	m_tagReads = 0;
	m_loadTags = g_guiSettings.GetBool("pictures.usetags");

	if (m_pProgressCallback)
		m_pProgressCallback->SetProgressMax(m_pVecItems->GetFileCount());
}

bool CPictureInfoLoader::LoadItem(CFileItem* pItem)
{
	if (m_pProgressCallback && !pItem->m_bIsFolder)
		m_pProgressCallback->SetProgressAdvance();

	pItem->SetCachedPictureThumb();

	if (pItem->m_bIsFolder || pItem->IsZIP() || pItem->IsRAR() || pItem->IsCBR() || pItem->IsCBZ() || pItem->IsInternetStream() || pItem->IsVideo())
		return false;

	if (pItem->HasPictureInfoTag())
		return true;

	// First check the cached item
	CFileItemPtr mapItem = (*m_mapFileItems)[pItem->GetPath()];

	if (mapItem && mapItem->m_dateTime==pItem->m_dateTime && mapItem->HasPictureInfoTag())
	{
		// Query map if we previously cached the file on HD
		*pItem->GetPictureInfoTag() = *mapItem->GetPictureInfoTag();
		pItem->SetThumbnailImage(mapItem->GetThumbnailImage());
		return true;
	}

	if (m_loadTags)
	{
		// Nothing found, load tag from file
		pItem->GetPictureInfoTag()->Load(pItem->GetPath());
		m_tagReads++;
	}

	return true;
}

void CPictureInfoLoader::OnLoaderFinish()
{
	// Cleanup cache loaded from HD
	m_mapFileItems->Clear();

	// Save loaded items to HD
	if (!m_bStop && m_tagReads > 0)
		m_pVecItems->Save();
}

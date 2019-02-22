#include "utils\stdafx.h"
#include "utils\log.h"

#include "BackgroundInfoLoader.h"

CBackgroundInfoLoader::CBackgroundInfoLoader()
{
	m_bRunning = false;
}

CBackgroundInfoLoader::~CBackgroundInfoLoader()
{
}

void CBackgroundInfoLoader::OnStartup()
{
	m_bRunning = true;
}

void CBackgroundInfoLoader::Process()
{
	try
	{
		CFileItemList& vecItems = (*m_pVecItems);

		if (vecItems.Size() <= 0)
			return;

		OnLoaderStart();

		for (int i = 0; i < (int)vecItems.Size(); ++i)
		{
			CFileItem* pItem = vecItems[i];

			// Ask the callback if we should abort
//			if(m_pProgressCallback && m_pProgressCallback->Abort()) //TODO
//				m_bStop=true;

			if(m_bStop)
				break;

			// Load the item
			if(!LoadItem(pItem))
				continue;

			// Notify observer a item
			// is loaded.
//			if (m_pObserver)
//				m_pObserver->OnItemLoaded(pItem);  //TODO
		}

		OnLoaderFinish();
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "BackgroundInfoLoader thread: Unhandled exception");
	}
}

void CBackgroundInfoLoader::OnExit()
{
	m_bRunning = false;
}

void CBackgroundInfoLoader::Load(CFileItemList& items)
{
	m_pVecItems = &items;

	StopThread();
	Create();

	m_bRunning = true;
}

bool CBackgroundInfoLoader::IsLoading()
{
	return m_bRunning;
}
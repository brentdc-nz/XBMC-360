#include "InfoLoader.h"
#include "JobManager.h"
#include "TimeUtils.h"
#include "guilib\LocalizeStrings.h"

CInfoJob::CInfoJob(CInfoLoader *loader)
{
	m_loader = loader;
}

bool CInfoJob::DoWork()
{
	if (m_loader)
		return m_loader->DoWork();

	return false;
}

CInfoLoader::CInfoLoader(unsigned int timeToRefresh)
{
	m_refreshTime = 0;
	m_timeToRefresh = timeToRefresh;
	m_busy = false;
}

CInfoLoader::~CInfoLoader()
{
}

void CInfoLoader::OnJobComplete(unsigned int jobID, bool success, CJob *job)
{
	m_refreshTime = CTimeUtils::GetFrameTime() + m_timeToRefresh;
	m_busy = false;
}

CStdString CInfoLoader::GetInfo(int info)
{
	// Refresh if need be
	if (m_refreshTime < CTimeUtils::GetFrameTime() && !m_busy)
	{
		// Queue up the job
		m_busy = true;
		CJobManager::GetInstance().AddJob(new CInfoJob(this), this);
	}
	
	if (m_busy && CTimeUtils::GetFrameTime() - m_refreshTime > 1000)
	{
		return BusyInfo(info);
	}

	return TranslateInfo(info);
}

CStdString CInfoLoader::BusyInfo(int info) const
{
	return g_localizeStrings.Get(503);
}

void CInfoLoader::Refresh()
{
	m_refreshTime = CTimeUtils::GetFrameTime();
}
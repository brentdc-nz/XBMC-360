#pragma once

#include "utils\StdString.h"
#include "Job.h"

class CInfoLoader;

class CInfoJob : public CJob
{
public:
	CInfoJob(CInfoLoader *loader);
	virtual bool DoWork();

private:
	CInfoLoader *m_loader; 
};

class CInfoLoader : public IJobCallback
{
public:
	CInfoLoader(unsigned int timeToRefresh = 5 * 60 * 1000);
	virtual ~CInfoLoader();

	CStdString GetInfo(int info);
	void Refresh();
  
	virtual void OnJobComplete(unsigned int jobID, bool success, CJob *job);
	virtual bool DoWork() = 0;

protected:
	virtual CStdString TranslateInfo(int info) const = 0;
	virtual CStdString BusyInfo(int info) const;

private:
	unsigned int m_refreshTime;
	unsigned int m_timeToRefresh;
	bool m_busy;
};
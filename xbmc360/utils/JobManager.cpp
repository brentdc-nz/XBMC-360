#include "JobManager.h"
#include <algorithm>
#include "utils\SingleLock.h"
#include "utils\log.h"

using namespace std;

bool CJob::ShouldCancel(unsigned int progress, unsigned int total) const
{
	if (m_callback)
		return m_callback->OnJobProgress(progress, total, this);
	
	return false;
}

CJobWorker::CJobWorker(CJobManager *manager)
{
	m_jobManager = manager;
	Create(true); // Start work immediately, and kill ourselves when we're done
}

CJobWorker::~CJobWorker()
{
	// While we should already be removed from the job manager, if an exception
	// occurs during processing that we haven't caught, we may skip over that step.
	// Thus, before we go out of scope, ensure the job manager knows we're gone.
	m_jobManager->RemoveWorker(this);
	
	if(!IsAutoDelete())
		StopThread();
}

void CJobWorker::Process()
{
	SetPriority(/*GetMinPriority()*/THREAD_PRIORITY_NORMAL); // Has issues any lower on the Xbox 360 CPU
	
	while (true)
	{
		// Request an item from our manager (this call is blocking)
		CJob *job = m_jobManager->GetNextJob(this);

		if (!job)
			break;

		bool success = false;

		try
		{
			success = job->DoWork();
		}
		catch (...)
		{
			CLog::Log(LOGERROR, "%s error processing job %s", __FUNCTION__, job->GetType());
		}

		m_jobManager->OnJobComplete(success, job);
	}
}

void CJobQueue::CJobPointer::CancelJob()
{
	CJobManager::GetInstance().CancelJob(m_id);
	m_id = 0;
}

CJobQueue::CJobQueue(bool lifo, unsigned int jobsAtOnce, CJob::PRIORITY priority)
	: m_jobsAtOnce(jobsAtOnce), m_priority(priority), m_lifo(lifo)
{
}

CJobQueue::~CJobQueue()
{
	CancelJobs();
}

void CJobQueue::OnJobComplete(unsigned int jobID, bool success, CJob *job)
{
	CSingleLock lock(m_section);
	
	// Check if this job is in our processing list
	Processing::iterator i = find(m_processing.begin(), m_processing.end(), job);
	
	if (i != m_processing.end())
		m_processing.erase(i);

	// Request a new job be queued
	QueueNextJob();
}

void CJobQueue::CancelJob(const CJob *job)
{
	CSingleLock lock(m_section);
	Processing::iterator i = find(m_processing.begin(), m_processing.end(), job);
	
	if (i != m_processing.end())
	{
		i->CancelJob();
		m_processing.erase(i);
		return;
	}
	
	Queue::iterator j = find(m_jobQueue.begin(), m_jobQueue.end(), job);
	
	if (j != m_jobQueue.end())
	{
		j->FreeJob();
		m_jobQueue.erase(j);
	}
}

void CJobQueue::AddJob(CJob *job)
{
	CSingleLock lock(m_section);

	// Check if we have this job already.  If so, we're done.
	if (find(m_jobQueue.begin(), m_jobQueue.end(), job) != m_jobQueue.end() ||
		find(m_processing.begin(), m_processing.end(), job) != m_processing.end())
	{
		delete job;
		return;
	}

	if (m_lifo)
		m_jobQueue.push_back(CJobPointer(job));
	else
		m_jobQueue.push_front(CJobPointer(job));
	
	QueueNextJob();
}

void CJobQueue::QueueNextJob()
{
	CSingleLock lock(m_section);
	
	if (m_jobQueue.size() && m_processing.size() < m_jobsAtOnce)
	{
		CJobPointer &job = m_jobQueue.back();
		job.m_id = CJobManager::GetInstance().AddJob(job.m_job, this, m_priority);
		m_processing.push_back(job);
		m_jobQueue.pop_back();
	}
}

void CJobQueue::CancelJobs()
{
	CSingleLock lock(m_section);
	for_each(m_processing.begin(), m_processing.end(), mem_fun_ref(&CJobPointer::CancelJob));
	for_each(m_jobQueue.begin(), m_jobQueue.end(), mem_fun_ref(&CJobPointer::FreeJob));
	m_jobQueue.clear();
	m_processing.clear();
}

CJobManager &CJobManager::GetInstance()
{
	static CJobManager sJobManager;
	return sJobManager;
}

CJobManager::CJobManager()
{
	m_jobCounter = 0;
	m_running = true;
}

void CJobManager::CancelJobs()
{
	CSingleLock lock(m_section);
	m_running = false;

	// Clear any pending jobs
	for (unsigned int priority = CJob::PRIORITY_LOW; priority <= CJob::PRIORITY_HIGH; ++priority)
	{
		for_each(m_jobQueue[priority].begin(), m_jobQueue[priority].end(), mem_fun_ref(&CWorkItem::FreeJob));
		m_jobQueue[priority].clear();
	}

	// Cancel any callbacks on jobs still processing
	for_each(m_processing.begin(), m_processing.end(), mem_fun_ref(&CWorkItem::Cancel));

	// Tell our workers to finish
	while (m_workers.size())
	{
		lock.Leave();
		m_jobEvent.Set();
		Sleep(0); // Yield after setting the event to give the workers some time to die
		lock.Enter();
	}
}

CJobManager::~CJobManager()
{
}

unsigned int CJobManager::AddJob(CJob *job, IJobCallback *callback, CJob::PRIORITY priority)
{
	CSingleLock lock(m_section);

	// Create a work item for this job
	CWorkItem work(job, m_jobCounter++, callback);
	m_jobQueue[priority].push_back(work);

	StartWorkers(priority);
	return work.m_id;
}

void CJobManager::CancelJob(unsigned int jobID)
{
	CSingleLock lock(m_section);

	// Check whether we have this job in the queue
	for (unsigned int priority = CJob::PRIORITY_LOW; priority <= CJob::PRIORITY_HIGH; ++priority)
	{
		JobQueue::iterator i = find(m_jobQueue[priority].begin(), m_jobQueue[priority].end(), jobID);
		
		if (i != m_jobQueue[priority].end())
		{
			delete i->m_job;
			m_jobQueue[priority].erase(i);
			return;
		}
	}
	
	// or if we're processing it
	Processing::iterator it = find(m_processing.begin(), m_processing.end(), jobID);
	
	if (it != m_processing.end())
		it->m_callback = NULL; // Job is in progress, so only thing to do is to remove callback
}

void CJobManager::StartWorkers(CJob::PRIORITY priority)
{
	CSingleLock lock(m_section);

	// Check how many free threads we have
	if (m_processing.size() >= GetMaxWorkers(priority))
		return;

	// Do we have any sleeping threads?
	if (m_processing.size() < m_workers.size())
	{
		m_jobEvent.Set();
		return;
	}

	// Everyone is busy - we need more workers
	m_workers.push_back(new CJobWorker(this));
}

CJob *CJobManager::PopJob()
{
	CSingleLock lock(m_section);
	
	for (int priority = CJob::PRIORITY_HIGH; priority >= CJob::PRIORITY_LOW; --priority)
	{
		if (m_jobQueue[priority].size() && m_processing.size() < GetMaxWorkers(CJob::PRIORITY(priority)))
		{
			CWorkItem job = m_jobQueue[priority].front();

			// Skip adding any paused types
			if (priority <= CJob::PRIORITY_LOW)
			{
				std::vector<std::string>::iterator i = find(m_pausedTypes.begin(), m_pausedTypes.end(), job.m_job->GetType());

				if (i != m_pausedTypes.end())
					return NULL;
			}

			m_jobQueue[priority].pop_front();
		
			// Add to the processing vector
			m_processing.push_back(job);
			job.m_job->m_callback = this;
			return job.m_job;
		}
	}
	return NULL;
}

void CJobManager::Pause(const std::string &pausedType)
{
	CSingleLock lock(m_section);

	// Just push it in so we get ref counting,
	// the queue will resume when all Pause requests
	// for a given type have been UnPaused.
	m_pausedTypes.push_back(pausedType);
}

void CJobManager::UnPause(const std::string &pausedType)
{
	CSingleLock lock(m_section);
	
	std::vector<std::string>::iterator i = find(m_pausedTypes.begin(), m_pausedTypes.end(), pausedType);
	
	if (i != m_pausedTypes.end())
		m_pausedTypes.erase(i);
}

bool CJobManager::IsPaused(const std::string &pausedType)
{
	CSingleLock lock(m_section);
	
	std::vector<std::string>::iterator i = find(m_pausedTypes.begin(), m_pausedTypes.end(), pausedType);
	
	return (i != m_pausedTypes.end());
}

int CJobManager::IsProcessing(const std::string &pausedType)
{
	int jobsMatched = 0;
	CSingleLock lock(m_section);
	
	for(Processing::iterator it = m_processing.begin(); it < m_processing.end(); it++)
	{
		if (pausedType == std::string(it->m_job->GetType()))
			jobsMatched++;
	}

	return jobsMatched;
}

CJob *CJobManager::GetNextJob(const CJobWorker *worker)
{
	CSingleLock lock(m_section);
	
	while (m_running)
	{
		// Grab a job off the queue if we have one
		CJob *job = PopJob();
		
		if (job)
			return job;
		
		// No jobs are left - sleep for 30 seconds to allow new jobs to come in
		lock.Leave();
		bool newJob = m_jobEvent.WaitMSec(30000);
		lock.Enter();
		
		if (!newJob)
			break;
	}
	
	// Ensure no jobs have come in during the period after
	// timeout and before we held the lock
	CJob *job = PopJob();

	if (job)
		return job;
	
	// Have no jobs
	RemoveWorker(worker);
	return NULL;
}

bool CJobManager::OnJobProgress(unsigned int progress, unsigned int total, const CJob *job) const
{
	CSingleLock lock(m_section);
	
	// Find the job in the processing queue, and check whether it's cancelled (no callback)
	Processing::const_iterator i = find(m_processing.begin(), m_processing.end(), job);
	
	if (i != m_processing.end())
	{
		CWorkItem item(*i);
		lock.Leave(); // Leave section prior to call
		
		if (item.m_callback)
		{
			item.m_callback->OnJobProgress(item.m_id, progress, total, job);
			return false;
		}
	}
	return true; // couldn't find the job, or it's been cancelled
}

void CJobManager::OnJobComplete(bool success, CJob *job)	
{
	CSingleLock lock(m_section);
	
	// Remove the job from the processing queue
	Processing::iterator i = find(m_processing.begin(), m_processing.end(), job);
	
	if (i != m_processing.end())
	{
		// Tell any listeners we're done with the job, then delete it
		CWorkItem item(*i);
		lock.Leave();
		
		try
		{
			if (item.m_callback)
				item.m_callback->OnJobComplete(item.m_id, success, item.m_job);
		}
		catch (...)
		{
			CLog::Log(LOGERROR, "%s error processing job %s", __FUNCTION__, item.m_job->GetType());
		}

		lock.Enter();
		Processing::iterator j = find(m_processing.begin(), m_processing.end(), job);
		
		if (j != m_processing.end())
			m_processing.erase(j);
		
		lock.Leave();
		item.FreeJob();
	}
}

void CJobManager::RemoveWorker(const CJobWorker *worker)
{
	CSingleLock lock(m_section);
	
	// Remove our worker
	Workers::iterator i = find(m_workers.begin(), m_workers.end(), worker);
	
	if (i != m_workers.end())
		m_workers.erase(i); // Workers auto-delete
}

unsigned int CJobManager::GetMaxWorkers(CJob::PRIORITY priority) const
{
	static const unsigned int max_workers = 5;
	return max_workers - (CJob::PRIORITY_HIGH - priority);
}
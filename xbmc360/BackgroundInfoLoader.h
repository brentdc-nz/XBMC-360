#ifndef H_CBACKGROUNDINFOLOADER
#define H_CBACKGROUNDINFOLOADER

#include "utils/Thread.h"
#include "IProgressCallback.h"
#include "utils/CriticalSection.h"

#include <vector>

class CFileItem;
#ifdef _HAS_BOOST
typedef boost::shared_ptr<CFileItem> CFileItemPtr;
#else
typedef std::shared_ptr<CFileItem> CFileItemPtr;
#endif
class CFileItemList;

class IBackgroundLoaderObserver
{
public:
	virtual ~IBackgroundLoaderObserver() {}
	virtual void OnItemLoaded(CFileItem* pItem) = 0;
};

class CBackgroundInfoLoader : public IRunnable
{
public:
	CBackgroundInfoLoader(int nThreads = -1);
	virtual ~CBackgroundInfoLoader();

	void Load(CFileItemList& items);
	bool IsLoading();
	virtual void Run();
	void SetObserver(IBackgroundLoaderObserver* pObserver);
	void SetProgressCallback(IProgressCallback* pCallback);
	virtual bool LoadItem(CFileItem* pItem) { return false; };

	void StopThread(); // Will actually stop all worker threads
	void StopAsync();  // Will ask loader to stop as soon as possible, but not block

	void SetNumOfWorkers(int nThreads); // -1 Means auto compute num of required threads

protected:
	virtual void OnLoaderStart() {};
	virtual void OnLoaderFinish() {};

	CFileItemList *m_pVecItems;
	std::vector<CFileItemPtr> m_vecItems; // FileItemList would delete the items and we only want to keep a reference.
	CCriticalSection m_lock;

	bool m_bStartCalled;
	volatile bool m_bStop;
	int  m_nRequestedThreads;
	int  m_nActiveThreads;

	IBackgroundLoaderObserver* m_pObserver;
	IProgressCallback* m_pProgressCallback;

	std::vector<CThread *> m_workers;
};

#endif //H_CBACKGROUNDINFOLOADER
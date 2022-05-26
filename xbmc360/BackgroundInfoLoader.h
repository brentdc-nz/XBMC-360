#ifndef H_CBACKGROUNDINFOLOADER
#define H_CBACKGROUNDINFOLOADER

#include "utils\Thread.h"
#include "FileItem.h"

class IBackgroundLoaderObserver
{
public:
	virtual void OnItemLoaded(CFileItem* pItem) = 0;
};

class CBackgroundInfoLoader : public CThread
{
public:
	CBackgroundInfoLoader();
	virtual ~CBackgroundInfoLoader();

	virtual void OnStartup();
	virtual void Process();
	virtual void OnExit();
	void Load(CFileItemList& items);

	virtual bool LoadItem(CFileItem* pItem) = 0;
	bool IsLoading();

protected:
	virtual void OnLoaderStart() {};
	virtual void OnLoaderFinish() {};

protected:
	CFileItemList* m_pVecItems;
	bool m_bRunning;
};

#endif //H_CBACKGROUNDINFOLOADER
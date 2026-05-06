#ifndef H_CASYNCFILECOPY
#define H_CASYNCFILECOPY

#include "utils\Thread.h"
#include "filesystem\File.h"

class CAsyncFileCopy : public CThread, public IFileCallback
{
public:
	CAsyncFileCopy();
	virtual ~CAsyncFileCopy();

	bool Copy(const CStdString &from, const CStdString &to, const CStdString &heading);
	virtual bool OnFileCallback(void *pContext, int ipercent, float avgSpeed);

protected:
	virtual void Process();

private:
	volatile int m_percent;
	volatile float m_speed;
	volatile bool m_cancelled;
	volatile bool m_running;

	bool m_succeeded;
	CStdString m_from;
	CStdString m_to;
	CEvent m_event;
};

#endif //H_CASYNCFILECOPY

#ifndef H_CIDLETHREAD
#define H_CIDLETHREAD

#include "Thread.h" 

class CIdleThread : public CThread
{
public:
	CIdleThread();
	virtual ~CIdleThread();

	virtual void OnStartup();
	virtual void OnExit();
	virtual void Process();
};

#endif //H_CIDLETHREAD
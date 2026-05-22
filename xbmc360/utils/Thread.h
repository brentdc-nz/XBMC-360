//
// Thread.h: interface for the CThread class.
//

#if !defined(AFX_THREAD_H__ACFB7357_B961_4AC1_9FB2_779526219817__INCLUDED_) && !defined(AFX_THREAD_H__67621B15_8724_4B5D_9343_7667075C89F2__INCLUDED_)
#define AFX_THREAD_H__ACFB7357_B961_4AC1_9FB2_779526219817__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Event.h"

typedef unsigned long ThreadIdentifier;

class IRunnable
{
public:
	virtual void Run()=0;
	virtual ~IRunnable() {}
};

#ifdef CTHREAD
#undef CTHREAD
#endif

class CThread
{
public:
	CThread();
	CThread(IRunnable* pRunnable);
	virtual ~CThread();
	void Create(bool bAutoDelete = false, unsigned stacksize = 0);
	unsigned long ThreadId() const;
	bool WaitForThreadExit(DWORD dwMilliseconds);
	DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);
	void Sleep(DWORD dwMilliseconds);
	bool SetPriority(const int iPriority);
	void SetName(LPCTSTR szThreadName);
	HANDLE ThreadHandle();
	operator HANDLE();
	operator const HANDLE() const;
	bool IsAutoDelete() const;
	bool IsRunning();
	virtual void StopThread(bool bWait = true);

	// Returns the relative cpu usage of this thread since last call
	float GetRelativeUsage();
	bool IsCurrentThread() const;
	int GetMinPriority(void);
	int GetMaxPriority(void);
	int GetNormalPriority(void);
	static bool IsCurrentThread(const ThreadIdentifier tid);
	static ThreadIdentifier GetCurrentThreadId();

protected:
	virtual void OnStartup(){};
	virtual void OnExit(){};
	virtual void Process();
	volatile bool m_bStop;
	HANDLE m_ThreadHandle;

private:
	bool m_bAutoDelete;
	HANDLE m_StopEvent;
	DWORD m_dwThreadId;
	IRunnable* m_pRunnable;

	unsigned __int64 m_iLastUsage;
	unsigned __int64 m_iLastTime;
	float m_fLastUsage;

	static DWORD WINAPI staticThread(LPVOID* data);
};

#endif // !defined(AFX_THREAD_H__ACFB7357_B961_4AC1_9FB2_779526219817__INCLUDED_)

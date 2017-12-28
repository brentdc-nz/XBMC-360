#include <xtl.h>
#include "lib.h"

#define MAX_SEM_COUNT 10

HANDLE LIB_CreateSemaphore(unsigned int initial_value){
	HANDLE hSemaphore;
	hSemaphore = CreateSemaphore( 
        NULL,           // default security attributes
        initial_value,  // initial count
        MAX_SEM_COUNT,  // maximum count
        NULL
	);          // unnamed semaphore
	return hSemaphore;
}

int LIB_SemPost(HANDLE sem){
	if(ReleaseSemaphore(sem, 1, NULL)==FALSE){
		return -1;
	}
	return 0;
}

HANDLE LIB_CreateThread(int (*fn)(void *), void *data){
	DWORD dwThreadId = 0;
	HANDLE hThread = CreateThread(NULL, 0, fn, data, 0, &dwThreadId);
	
	return hThread;
}

void LIB_WaitThread(HANDLE thread, int *status){
	WaitForSingleObject(thread, INFINITE);
	CloseHandle(thread);
}

int LIB_SemWaitTimeout(HANDLE sem, unsigned int timeout){
	return 0;
}

int LIB_SemWait(HANDLE sem)
{
	return WaitForSingleObject(sem, INFINITE);
}
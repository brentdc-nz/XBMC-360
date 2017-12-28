#include <xtl.h>

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//	Thread 
HANDLE LIB_CreateThread(int (*fn)(void *), void *data);
void LIB_WaitThread(HANDLE thread, int *status);

//	Semaphore
HANDLE LIB_CreateSemaphore(unsigned int initial_value);
int LIB_SemPost(HANDLE sem);
int LIB_SemWaitTimeout(HANDLE sem, unsigned int timeout);
int LIB_SemWait(HANDLE sem);

#ifdef __cplusplus
}
#endif
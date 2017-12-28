

HANDLE LIB_CreateSemaphore(Uint32 initial_value){
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
	CloseHandle(thread->handle);
}

int LIB_SemWaitTimeout(HANDLE sem, Uint32 timeout)
	return 0;
}

int LIB_SemWait(HANDLE sem){
	return WaitForSingleObject(sem, INFINITE))
}
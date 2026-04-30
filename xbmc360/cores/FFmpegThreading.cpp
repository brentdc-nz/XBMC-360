#include "utils\stdafx.h"
#include "utils\Log.h"
#include "cores\FFmpegThreading.h"

#define PTW32_STATIC_LIB
#include <pthread.h>

extern "C" {
#include "libavcodec\avcodec.h"
}

// pthreads-win32 internal init/term for static lib builds (see README.Xbox)
extern "C" int ptw32_processInitialize(void);
extern "C" void ptw32_processTerminate(void);

// pthreads-win32 function pointer that must be set manually for static builds.
// pthread_win32_process_attach_np (which normally sets this) is compiled out
// by PTW32_STATIC_LIB. Without it, any pthreads function using
// PTW32_INTERLOCKED_COMPARE_EXCHANGE (e.g. pthread_mutex_destroy) crashes
// by calling through a NULL function pointer.
extern "C" long (WINAPI * ptw32_interlocked_compare_exchange)(long*, long, long);

// Wrapper needed because InterlockedCompareExchange is a compiler intrinsic
// on Xbox 360 (PowerPC) — you can't take its address directly.
static long WINAPI xbox360_InterlockedCompareExchange(long *dest, long exchange, long comparand)
{
	return InterlockedCompareExchange(dest, exchange, comparand);
}

// FFmpeg lock manager callback for thread-safe avcodec_open/close
static int ffmpeg_lockmgr_cb(void **mutex, enum AVLockOp op)
{
	CRITICAL_SECTION **cs = (CRITICAL_SECTION **)mutex;
	switch (op)
	{
	case AV_LOCK_CREATE:
		*cs = (CRITICAL_SECTION *)av_malloc(sizeof(CRITICAL_SECTION));
		if (!*cs) return 1;
		InitializeCriticalSection(*cs);
		return 0;
	case AV_LOCK_OBTAIN:
		EnterCriticalSection(*cs);
		return 0;
	case AV_LOCK_RELEASE:
		LeaveCriticalSection(*cs);
		return 0;
	case AV_LOCK_DESTROY:
		DeleteCriticalSection(*cs);
		av_free(*cs);
		*cs = NULL;
		return 0;
	}
	return 1;
}

void ffmpeg_threading_init(void)
{
	ptw32_processInitialize();
	ptw32_interlocked_compare_exchange = xbox360_InterlockedCompareExchange;

	if (av_lockmgr_register(ffmpeg_lockmgr_cb))
		CLog::Log(LOGERROR, "ffmpeg_threading_init - Failed to register FFmpeg lock manager!");
}

void ffmpeg_threading_deinit(void)
{
	av_lockmgr_register(NULL);
	ptw32_processTerminate();
}

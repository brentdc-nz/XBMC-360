#ifndef FFMPEG_THREADING_H
#define FFMPEG_THREADING_H

// Xbox 360: Initializes pthreads-win32 static library and FFmpeg lock manager.
// Must be called once at startup before any FFmpeg or pthreads usage.
void ffmpeg_threading_init(void);

// Xbox 360: Tears down FFmpeg lock manager and pthreads-win32.
// Must be called once at shutdown after all FFmpeg usage is complete.
void ffmpeg_threading_deinit(void);

#endif // FFMPEG_THREADING_H

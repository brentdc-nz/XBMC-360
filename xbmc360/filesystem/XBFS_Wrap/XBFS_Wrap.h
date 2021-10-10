#ifndef XBFS_WRAP_H
#define XBFS_WRAP_H

#include "..\File.h"

#ifdef __cplusplus
extern "C" {
#endif

int XBFS_open(const char* szFileName, int iMode);
int XBFS_read(int fd, void* buffer, unsigned int uiSize);
int XBFS_write(int fd, const void* buffer, unsigned int uiSize);
long XBFS_lseek(int fd, long lPos, int iWhence);
int XBFS_fstat(int fd, struct stat* buffer);
int XBFS_stat(const char *path, struct _stat *buffer);
int XBFS_close(int fd);
int XBFS_fclose(FILE * stream);

#ifdef __cplusplus
}
#endif

#endif //XBFS_WRAP_H
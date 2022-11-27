#include "XBFS_Wrap.h"
#include "fcntl.h"
#include "utils\Log.h"
#include "EmuFileWrapper.h"
#include "io.h"
#include <sys/stat.h>
#include "utils\Util.h"

CEmuFileWrapper g_emuFileWrapper;

#pragma warning(disable:4244) // TODO: Fix rather than disable

int XBFS_open(const char* szFileName, int iMode)
{
	XFILE::CFile* pFile = new XFILE::CFile();
	bool bBinary = false;

	if (iMode & O_BINARY)
		bBinary = true;

	bool bWrite = false;

	if(iMode & O_RDWR || iMode & O_WRONLY)
		bWrite = true;

	bool bOverwrite=false;

	if (iMode & _O_TRUNC)
		bOverwrite = true;

    // Currently always overwrites
	if((bWrite && pFile->OpenForWrite(szFileName, bOverwrite)) || pFile->Open(szFileName, bBinary) )
	{
		EmuFileObject* object = g_emuFileWrapper.RegisterFileObject(pFile);
		if(object == NULL)
		{
			VERIFY(0);
			pFile->Close();
			delete pFile;
			return -1;
		}
		return g_emuFileWrapper.GetDescriptorByStream(&object->file_emu);
	}
	delete pFile;
	return -1;
}

int XBFS_read(int fd, void* buffer, unsigned int uiSize)
{
    XFILE::CFile* pFile = g_emuFileWrapper.GetFileXbmcByDescriptor(fd);
    if (pFile != NULL)
    {
		unsigned int iRslt = pFile->Read(buffer, uiSize, 0);
       return iRslt;
    }
    else if (!IS_STD_DESCRIPTOR(fd))
    {
      // it might be something else than a file, or the file is not emulated
      // let the operating system handle it
      return read(fd, buffer, uiSize);
    }
    CLog::Log(LOGERROR, "emulated function " __FUNCTION__ " failed");
    return -1;
}

int XBFS_write(int fd, const void* buffer, unsigned int uiSize)
{
	XFILE::CFile* pFile = g_emuFileWrapper.GetFileXbmcByDescriptor(fd);
	
	if (pFile != NULL)
		return pFile->Write(buffer, uiSize);

	else if (!IS_STD_DESCRIPTOR(fd))
	{
		// It might be something else than a file, or the file 
		// is not emulated let the operating system handle it
		return write(fd, buffer, uiSize);
	}

    CLog::Log(LOGERROR, "%s emulated function failed",  __FUNCTION__);
    return -1;
}

__int64 XBFS_lseeki64(int fd, __int64 lPos, int iWhence)
{
    XFILE::CFile* pFile = g_emuFileWrapper.GetFileXbmcByDescriptor(fd);
    if (pFile != NULL)
    {
      lPos = pFile->Seek(lPos, iWhence);
      return lPos;
    }
    else if (!IS_STD_DESCRIPTOR(fd))
    {
      // it might be something else than a file, or the file is not emulated
      // let the operating system handle it
      // not supported: return lseeki64(fd, lPos, iWhence);
      CLog::Log(LOGWARNING, "msvcrt.dll: dll_lseeki64 called, TODO: add 'int64 -> long' type checking");      //warning
      return (__int64)lseek(fd, (long)lPos, iWhence);
    }
    CLog::Log(LOGERROR, "emulated function " __FUNCTION__ " failed");
    return (__int64)-1;
}

long XBFS_lseek(int fd, long lPos, int iWhence)
{
    if (g_emuFileWrapper.DescriptorIsEmulatedFile(fd))
    {
	  long iSeekPos = XBFS_lseeki64(fd, (__int64)lPos, iWhence);
      return iSeekPos;
    }
    else if (!IS_STD_DESCRIPTOR(fd))
    {
      // it might be something else than a file, or the file is not emulated
      // let the operating system handle it
      return lseek(fd, lPos, iWhence);
    }
    CLog::Log(LOGERROR, "emulated function " __FUNCTION__ " failed");
    return -1;
}

int XBFS_fstat(int fd, struct stat* buffer)
{
    XFILE::CFile* pFile = g_emuFileWrapper.GetFileXbmcByDescriptor(fd);
    if (pFile != NULL)
    {
      CLog::Log(LOGINFO, "Stating open file");
    
      __int64 size = pFile->GetLength();
      if (size <= LONG_MAX)
        buffer->st_size = (_off_t)size;
      else
      {
        buffer->st_size = 0;
        CLog::Log(LOGWARNING, "WARNING: File is larger than 32bit stat can handle, file size will be reported as 0 bytes");
      }
      buffer->st_mode = _S_IFREG;

      return 0;
    }
    else if (!IS_STD_DESCRIPTOR(fd))
    {
      return fstat(fd, buffer);
    }
    
    // fstat on stdin, stdout or stderr should fail
    // this is what python expects
    return -1;
}

//SLOW CODE SHOULD BE REVISED
int XBFS_stat(const char *path, struct _stat *buffer)
{
    CLog::Log(LOGINFO, "Stating file %s", path);

    if (!strnicmp(path, "shout://", 8)) // Don't stat shoutcast
      return -1;
    if (!strnicmp(path, "http://", 7)) // Don't stat http
      return -1;
    if (!strnicmp(path, "mms://", 6)) // Don't stat mms
      return -1;
    if (!_stricmp(path, "D:") || !_stricmp(path, "D:\\"))
    {
      buffer->st_mode = S_IFDIR;
      return 0;
    }
    if (!stricmp(path, "\\Device\\Cdrom0") || !stricmp(path, "\\Device\\Cdrom0\\"))
    {
      buffer->st_mode = _S_IFDIR;
      return 0;
    }

	struct __stat64 tStat;
	if (XFILE::CFile::Stat(path, &tStat) == 0)
	{
		CUtil::Stat64ToStat(buffer, &tStat);
		return 0;
	}

	return -1;
}

int XBFS_close(int fd)
  {
    XFILE::CFile* pFile = g_emuFileWrapper.GetFileXbmcByDescriptor(fd);
    if (pFile != NULL)
    {
      g_emuFileWrapper.UnRegisterFileObjectByDescriptor(fd);
      
      pFile->Close();
      delete pFile;
      return 0;
    }
    else if (!IS_STD_DESCRIPTOR(fd))
    {
      // it might be something else than a file, or the file is not emulated
      // let the operating system handle it
      return _close(fd);
    }
    CLog::Log(LOGERROR, "%s emulated function failed",  __FUNCTION__);
    return -1;
  }

int XBFS_fclose(FILE * stream)
{
    int fd = g_emuFileWrapper.GetDescriptorByStream(stream);
    if (fd >= 0)
    {
      return XBFS_close(fd);
    }
    else if (!IS_STD_STREAM(stream))
    {
      // it might be something else than a file, let the operating system handle it
      return fclose(stream);
    }
    CLog::Log(LOGERROR, "%s emulated function failed",  __FUNCTION__);
    return EOF;
}
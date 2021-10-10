#include "..\..\utils\stdafx.h"
#include "..\File.h"

#pragma once

#define __IS_STDIN_STREAM(stream)   (stream == stdin  || stream->_file == 0)
#define __IS_STDOUT_STREAM(stream)  (stream == stdout || stream->_file == 1)
#define __IS_STDERR_STREAM(stream)  (stream == stderr || stream->_file == 2)
#define IS_STDIN_STREAM(stream)     (stream != NULL && __IS_STDIN_STREAM(stream))
#define IS_STDOUT_STREAM(stream)    (stream != NULL && __IS_STDOUT_STREAM(stream))
#define IS_STDERR_STREAM(stream)    (stream != NULL && __IS_STDERR_STREAM(stream))

#define MAX_EMULATED_FILES    50
#define FILE_WRAPPER_OFFSET   0x00000100

#define IS_STD_STREAM(stream)       (stream != NULL && (__IS_STDIN_STREAM(stream) || __IS_STDOUT_STREAM(stream) || __IS_STDERR_STREAM(stream)))

#define IS_STDIN_DESCRIPTOR(fd)  (fd == 0)
#define IS_STDOUT_DESCRIPTOR(fd) (fd == 1)
#define IS_STDERR_DESCRIPTOR(fd) (fd == 2)

#define IS_STD_DESCRIPTOR(fd) (IS_STDIN_DESCRIPTOR(fd) || IS_STDOUT_DESCRIPTOR(fd) || IS_STDERR_DESCRIPTOR(fd))

typedef struct stEmuFileObject
{
	bool    used;
	FILE    file_emu;
	XFILE::CFile*  file_xbmc;
} EmuFileObject;
  
class CEmuFileWrapper
{
public:
	CEmuFileWrapper()
	{
		InitializeCriticalSection(&m_criticalSection);

		// Since we always use dlls we might just initialize it directly
		for(int i = 0; i < MAX_EMULATED_FILES; i++)
		{
			memset(&m_files[i], 0, sizeof(EmuFileObject));
			m_files[i].used = false;
			m_files[i].file_emu._file = -1;
		}
	}
  
	~CEmuFileWrapper()
	{
		DeleteCriticalSection(&m_criticalSection);
	}
  

	// Only to be called when shutting down xbmc	
	void CleanUp()
	{
		EnterCriticalSection(&m_criticalSection);
		for(int i = 0; i < MAX_EMULATED_FILES; i++)
		{
			if(m_files[i].used)
			{
				m_files[i].file_xbmc->Close();
				delete m_files[i].file_xbmc;
        
				memset(&m_files[i], 0, sizeof(EmuFileObject));
				m_files[i].used = false;
				m_files[i].file_emu._file = -1;
			}
		}
		LeaveCriticalSection(&m_criticalSection);
	}
  
	EmuFileObject* RegisterFileObject(XFILE::CFile* pFile)
	{
		EmuFileObject* object = NULL;
    
		EnterCriticalSection(&m_criticalSection);
    
		for(int i = 0; i < MAX_EMULATED_FILES; i++)
		{
			if(!m_files[i].used)
			{
				// Found a free location
				object = &m_files[i];
				object->used = true;
				object->file_xbmc = pFile;
				object->file_emu._file = (i + FILE_WRAPPER_OFFSET);
				break;
			}
		}
    
		LeaveCriticalSection(&m_criticalSection);
    
		return object;
	}
  
	void UnRegisterFileObjectByDescriptor(int fd)
	{
		int i = fd - FILE_WRAPPER_OFFSET;
		if(i >= 0 && i < MAX_EMULATED_FILES)
		{
			if(m_files[i].used)
			{
				EnterCriticalSection(&m_criticalSection);
        
				// We assume the emulated function alreay deleted the CFile object
				if(m_files[i].used)
				{
					memset(&m_files[i], 0, sizeof(EmuFileObject));
					m_files[i].used = false;
					m_files[i].file_emu._file = -1;
				}
        
				LeaveCriticalSection(&m_criticalSection);
			}
		}
	}
  
	void UnRegisterFileObjectByStream(FILE* stream)
	{
		if(stream != NULL)
		{
			return UnRegisterFileObjectByDescriptor(stream->_file);
		}
	}

	EmuFileObject* GetFileObjectByDescriptor(int fd)
	{
		int i = fd - FILE_WRAPPER_OFFSET;
		if(i >= 0 && i < MAX_EMULATED_FILES)
		{
			if(m_files[i].used)
			{
				return &m_files[i];
			}
		}
		return NULL;
	}
  
	EmuFileObject* GetFileObjectByStream(FILE* stream)
	{
		if(stream != NULL)
		{
			return GetFileObjectByDescriptor(stream->_file);
		}
	}
  
	XFILE::CFile* GetFileXbmcByDescriptor(int fd)
	{
		EmuFileObject* object = GetFileObjectByDescriptor(fd);
		if(object != NULL && object->used)
		{
			return object->file_xbmc;
		}
		return NULL;
	}

	XFILE::CFile* GetFileXbmcByStream(FILE* stream)
	{
		if(stream != NULL)
		{
			EmuFileObject* object = GetFileObjectByDescriptor(stream->_file);
			if(object != NULL && object->used)
			{
				return object->file_xbmc;
			}
		}
		return NULL;
	}
  
	int GetDescriptorByStream(FILE* stream)
	{
		if(stream != NULL)
		{
			int i = stream->_file - FILE_WRAPPER_OFFSET;
			if(i >= 0 && i < MAX_EMULATED_FILES)
			{
				return stream->_file;
			}
		}
		return -1;
	}

	FILE* GetStreamByDescriptor(int fd)
	{
		EmuFileObject* object = GetFileObjectByDescriptor(fd);
		if(object != NULL && object->used)
		{
			return &object->file_emu;
		}
		return NULL;
	}
  
	bool DescriptorIsEmulatedFile(int fd)
	{
		int i = fd - FILE_WRAPPER_OFFSET;
		if(i >= 0 && i < MAX_EMULATED_FILES)
		{
			return true;
		}
		return false;
	}

	bool StreamIsEmulatedFile(FILE* stream)
	{
		if(stream != NULL)
		{
			return DescriptorIsEmulatedFile(stream->_file);
		}
		return false;
	}
   
private:
	EmuFileObject m_files[MAX_EMULATED_FILES];
	CRITICAL_SECTION m_criticalSection;
	bool m_initialized;
};

extern CEmuFileWrapper g_emuFileWrapper;
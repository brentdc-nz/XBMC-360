#ifndef IDIRECTORY_H
#define IDIRECTORY_H

#include "utils\StdString.h"

class CFileItemList;

namespace XFILE
{
	enum DIR_CACHE_TYPE
	{
		DIR_CACHE_NEVER = 0, // Never cache this directory to memory
		DIR_CACHE_ONCE,      // Cache this directory to memory for each fetch (so that FileExists() checks are fast)
		DIR_CACHE_ALWAYS     // Always cache this directory to memory, so that each additional fetch of this folder will utilize the cache (until it's cleared)
	};

// ingroup filesystem 
// Interface to the directory on a file system

// This Interface is retrieved from CFactoryDirectory and can be used to 
// access the directories on a filesystem.
class IDirectory
{
public:
	IDirectory(void);
	virtual ~IDirectory(void);

	// Get the \e items of the directory \e strPath.
	// param strPath Directory to read.
	// param items Retrieves the directory entries.
	// return Returns \e true, if successfull.
	virtual bool GetDirectory(const CStdString& strPath, CFileItemList &items) = 0;

	// Create the directory
	// param strPath Directory to create.
	// return Returns \e true, if directory is created or if it already exists
	virtual bool Create(const char* strPath) { return false; }

	// Check for directory existence
	// param strPath Directory to check.
	// return Returns \e true, if directory exists
	virtual bool Exists(const char* strPath) { return false; }

	// Removes the directory 
	// param strPath Directory to remove.
	// return Returns \e false if not succesfull
	virtual bool Remove(const char* strPath) { return false; }

	// Whether this file should be listed
	// param strFile File to test.
	// return Returns \e true if the file should be listed
	virtual bool IsAllowed(const CStdString& strFile) const;


	// How this directory should be cached
	// param strPath Directory at hand.
	// return Returns the cache type.
	virtual DIR_CACHE_TYPE GetCacheType(const CStdString& strPath) const { return DIR_CACHE_ONCE; };

	void SetMask(const CStdString& strMask);
	void SetAllowPrompting(bool allowPrompting);
	void SetCacheDirectory(DIR_CACHE_TYPE cacheDirectory);
	void SetUseFileDirectories(bool useFileDirectories);
	void SetExtFileInfo(bool extFileInfo);

protected:
	CStdString m_strFileMask; // Holds the file mask specified by SetMask()
	bool m_allowPrompting; // If true, the directory handlers may prompt the user
	DIR_CACHE_TYPE m_cacheDirectory; // If !DIR_CACHE_NONE the directory is cached by g_directoryCache (defaults to DIR_CACHE_NONE)
	bool m_useFileDirectories; // If true the directory may allow file directories (defaults to false)
	bool m_extFileInfo; // If true the GetDirectory call can retrieve extra file information (defaults to true)
};

};
#endif //IDIRECTORY_H
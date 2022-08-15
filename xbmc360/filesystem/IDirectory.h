#include "..\FileItem.h"

#ifndef H_IDIRECTORY
#define H_IDIRECTORY

namespace DIRECTORY
{

class IDirectory
{
public:
	IDirectory(void);
	virtual ~IDirectory(void);

    // Get the items of the directory strPath
    // Param strPath Directory to read
    // Param items Retrieves the directory entries
    // Return Returns true, if successfull
	virtual bool GetDirectory(const CStdString& strPath, CFileItemList &items) = 0;
	bool IsAllowed(const CStdString& strFile);
	void SetMask(const CStdString& strMask);
	virtual bool Create(const char* strPath) { return false; }
	virtual bool Exists(const char* strPath) { return false; }
	virtual bool Remove(const char* strPath) { return false; }

protected:
	CStdString m_strFileMask; // Holds the file mask specified by SetMask()
	bool m_allowPrompting;    // If true, the directory handlers may prompt the user
	bool m_cacheDirectory;    // If true (default) the directory is cached by g_directoryCache
};

};
#endif //H_IDIRECTORY
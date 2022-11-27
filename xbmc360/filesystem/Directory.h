#ifndef H_CDIRECTORY
#define H_CDIRECTORY

#include "FileItem.h"
#include "IDirectory.h"

namespace XFILE
{

// Wrappers for IDirectory

class CDirectory
{
public:
	CDirectory(void);
	virtual ~CDirectory(void);

	static bool GetDirectory(const CStdString& strPath, CFileItemList &items, CStdString strMask="", bool bUseFileDirectories=true, bool allowPrompting=false, DIR_CACHE_TYPE cacheDirectory = DIR_CACHE_ONCE, bool extFileInfo=true);
	static bool Create(const CStdString& strPath);
	static bool Exists(const CStdString& strPath);
	static bool Remove(const CStdString& strPath);

	// Filter files that act like directories from the list, replacing them with their directory counterparts
	// param items The item list to filter
	// param mask  The mask to apply when filtering files
	static void FilterFileDirectories(CFileItemList &items, const CStdString &mask);
};
}

#endif //H_CDIRECTORY
#ifndef H_CDIRECTORY
#define H_CDIRECTORY

#include "../FileItem.h"

namespace DIRECTORY
{

// Wrappers for IDirectory

class CDirectory
{
public:
	CDirectory(void);
	virtual ~CDirectory(void);

	static bool GetDirectory(const CStdString& strPath, CFileItemList &items, CStdString strMask="", bool bUseFileDirectories=true, bool allowPrompting=false, bool cacheDirectory=false);
	static bool Create(const CStdString& strPath);
	static bool Exists(const CStdString& strPath);
	static bool Remove(const CStdString& strPath);
};

}
#endif //H_CDIRECTORY
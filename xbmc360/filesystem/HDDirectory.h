#ifndef H_CHDDDIRECTORY
#define H_CHDDDIRECTORY

#include "..\utils\StdString.h"
#include "..\FileItem.h"

namespace XFILE
{

class CHDDirectory
{
public:
	CHDDirectory(void);
	~CHDDirectory(void);

	bool GetDirectory(const CStdString& strPath, CFileItemList &items);
	bool Exists(const char* strPath);
};

}

#endif //H_CHDDDIRECTORY
#ifndef H_CFILEFACTORY
#define H_CFILEFACTORY

#include "FileBase.h"
#include "..\utils\StdString.h"
#include "..\URL.h"

namespace XFILE
{

class CFileFactory
{
public:
	CFileFactory();
	virtual ~CFileFactory();
	static CFileBase* CreateLoader(const CURL& url);
	static CFileBase* CreateLoader(const CStdString& strFileName);
};

}
#endif //H_CFILEFACTORY

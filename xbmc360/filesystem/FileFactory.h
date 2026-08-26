#ifndef H_CFILEFACTORY
#define H_CFILEFACTORY

#include "IFile.h"
#include "..\utils\StdString.h"
#include "..\URL.h"

namespace XFILE
{

class CFileFactory
{
public:
	CFileFactory();
	virtual ~CFileFactory();
	static IFile* CreateLoader(const CURL& url);
	static IFile* CreateLoader(const CStdString& strFileName);
};

}
#endif //H_CFILEFACTORY

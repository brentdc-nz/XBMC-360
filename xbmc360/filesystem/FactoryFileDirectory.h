#ifndef H_CFACTORYFILEDIRECTORY
#define H_CFACTORYFILEDIRECTORY

#include "FactoryFileDirectory.h"
#include "IFileDirectory.h"

namespace DIRECTORY
{
class CFactoryFileDirectory
{
public:
	CFactoryFileDirectory(void);
	virtual ~CFactoryFileDirectory(void);
	static IFileDirectory* Create(const CStdString& strPath, CFileItem* pItem, const CStdString& strMask="");
};

};
#endif //H_CFACTORYFILEDIRECTORY
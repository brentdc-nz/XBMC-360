#ifndef H_IFILEDIRECTORY
#define H_IFILEDIRECTORY

#include "IDirectory.h"

using namespace DIRECTORY;

namespace DIRECTORY
{

class IFileDirectory : public IDirectory
{
public:
	virtual ~IFileDirectory(void){};
	virtual bool ContainsFiles(const CStdString& strPath) = 0;
};

}
#endif //H_IFILEDIRECTORY
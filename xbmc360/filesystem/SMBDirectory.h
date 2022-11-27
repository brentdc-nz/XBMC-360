#ifndef H_CSMBDIRECTORY
#define H_CSMBDIRECTORY

#include "FileBase.h"
#include "iDirectory.h"
#include "FileSmb.h"
#include <fcntl.h>

namespace XFILE
{

class CSMBDirectory : public IDirectory
{
public:
	CSMBDirectory(void);
	virtual ~CSMBDirectory(void);

	virtual bool GetDirectory(const CStdString& strPath, CFileItemList &items);
	virtual bool Create(const char* strPath);
	virtual bool Exists(const char* strPath);
	virtual bool Remove(const char* strPath);

	int Open(const CURL &url);

private:
};
}

#endif //H_CSMBDIRECTORY
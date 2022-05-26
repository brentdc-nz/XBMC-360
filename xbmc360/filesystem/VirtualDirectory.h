#ifndef H_CVIRTUALDIRECTORY
#define H_CVIRTUALDIRECTORY

#include "IDirectory.h"
#include "..\MediaManager.h"

namespace DIRECTORY
{

class CVirtualDirectory : public IDirectory
{
public:
	CVirtualDirectory(void);
	virtual ~CVirtualDirectory(void);

	virtual bool GetDirectory(const CStdString& strPath, CFileItemList &items);
	virtual bool GetDirectory(const CStdString& strPath, CFileItemList &items, bool bUseFileDirectories);
	void SetShares(VECSOURCES& vecShares);
	void GetShares(VECSOURCES& shares) const;
	bool IsShare(const CStdString& strPath) const;

protected:
	VECSOURCES* m_vecShares;
};

};

#endif //H_CVIRTUALDIRECTORY
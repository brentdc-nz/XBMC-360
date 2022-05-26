#ifndef H_CMULTIPATHDIRECTORY
#define H_CMULTIPATHDIRECTORY

#include "IDirectory.h"
#include "..\utils\StdString.h"
#include "..\FileItem.h"

namespace DIRECTORY
{

class CMultiPathDirectory :	public IDirectory
{
public:
	CMultiPathDirectory(void);
	virtual ~CMultiPathDirectory(void);

	virtual bool GetDirectory(const CStdString& strPath, CFileItemList &items);
	static bool GetPaths(const CStdString& strPath, vector<CStdString>& vecPaths);
	static CStdString GetFirstPath(const CStdString &strPath);
	void MergeItems(CFileItemList &items);
	static CStdString ConstructMultiPath(const vector<CStdString> &vecPaths);

private:
	CStdString ConstructMultiPath(const CFileItemList& items, const vector<int> &stack);
};

};

#endif //H_CMULTIPATHDIRECTORY
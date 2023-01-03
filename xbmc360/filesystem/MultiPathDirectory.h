#ifndef H_CMULTIPATHDIRECTORY
#define H_CMULTIPATHDIRECTORY

#include "IDirectory.h"
#include "..\utils\StdString.h"
#include "..\FileItem.h"

#include "IDirectory.h"

namespace XFILE
{

class CMultiPathDirectory : public IDirectory
{
public:
	CMultiPathDirectory(void);
	virtual ~CMultiPathDirectory(void);
	virtual bool GetDirectory(const CStdString& strPath, CFileItemList &items);
	virtual bool Exists(const char* strPath);
	virtual bool Remove(const char* strPath);

	static CStdString GetFirstPath(const CStdString &strPath);
	static bool SupportsWriteFileOperations(const CStdString &strPath);
	static bool GetPaths(const CStdString& strPath, std::vector<CStdString>& vecPaths);
	static bool HasPath(const CStdString& strPath, const CStdString& strPathToFind);
	static CStdString ConstructMultiPath(const std::vector<CStdString> &vecPaths);

private:
	void MergeItems(CFileItemList &items);
	static void AddToMultiPath(CStdString& strMultiPath, const CStdString& strPath);
	CStdString ConstructMultiPath(const CFileItemList& items, const std::vector<int> &stack);
};

}

#endif //H_CMULTIPATHDIRECTORY
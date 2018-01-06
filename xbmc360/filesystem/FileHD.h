#ifndef H_CFILEHD
#define H_CFILEHD

#include "..\utils\StdString.h"

namespace XFILE
{

class CFileHD
{
public:
	CFileHD();
	~CFileHD();

	static bool Exists(const CStdString& strPath);
};

};

#endif //H_CFILEHD
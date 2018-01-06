#ifndef H_CFILE
#define H_CFILE

#include "..\utils\StdString.h"

namespace XFILE
{

class CFile
{
public:
	CFile(){};
	virtual ~CFile(){};


	static bool Exists(const CStdString& strFileName);

private:

};

};

#endif //H_CFILE
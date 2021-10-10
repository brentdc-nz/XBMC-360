#ifndef H_CDVDINPUTSTREAMFILE
#define H_CDVDINPUTSTREAMFILE

#include "DVDInputStream.h"
#include "..\..\..\filesystem\File.h"

class CDVDInputStreamFile : public CDVDInputStream
{
public:
	CDVDInputStreamFile();
	virtual ~CDVDInputStreamFile();

	virtual bool Open(const char* strFile/*, const std::string &content*/);
	virtual void Close();

	virtual bool IsEOF();
protected:
	XFILE::CFile* m_pFile;
	bool m_bEOF;
};

#endif //H_CDVDINPUTSTREAMFILE
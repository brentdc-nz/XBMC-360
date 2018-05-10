#ifndef H_CDVDINPUTSTREAMFILE
#define H_CDVDINPUTSTREAMFILE

#include "DVDInputStream.h"
#include "..\..\..\filesystem\File.h"

class CDVDInputStreamFile : public CDVDInputStream
{
public:
	CDVDInputStreamFile();
	virtual ~CDVDInputStreamFile();

	virtual bool Open(const char* strFile);
	virtual void Close();
};

#endif //H_CDVDINPUTSTREAMFILE
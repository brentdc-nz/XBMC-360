#ifndef CURL_H
#define CURL_H

#include "utils/StdString.h"

class CURL
{
public:
	CURL(const CStdString& strURL);
	CURL();
	~CURL();

	void Reset();
	const CStdString& GetProtocol() const;
	const CStdString& GetFileName() const;

private:
	void Parse(const CStdString& strURL);
	void SetProtocol(const CStdString& strProtocol);
	void SetFileName(const CStdString& strFileName);

	CStdString m_strProtocol;
	CStdString m_strFileName;
};

#endif //CURL_H
#ifndef CURL_H
#define CURL_H

#include "utils\StdString.h"

class CURL
{
public:
	CURL(const CStdString& strURL);
	CURL();
	~CURL();

	void Reset();
	void SetFileName(const CStdString& strFileName);
	const CStdString& GetProtocol() const;
	const CStdString& GetFileName() const;

	void GetURL(CStdString& strURL) const;
	inline const char GetDirectorySeparator() const;
	void GetURLWithoutUserDetails(CStdString& strURL) const;
	void GetURLWithoutFilename(CStdString& strURL) const;
	bool IsLocal() const;

private:
	void Parse(const CStdString& strURL);
	void SetProtocol(const CStdString& strProtocol);

	CStdString m_strFileType;
	CStdString m_strProtocol;
	CStdString m_strFileName;
};

#endif //CURL_H
#ifndef H_CHTTPCLEINT
#define H_CHTTPCLEINT

#include <string>

// A very basic crude http client as temporary stand-in solution till we have somthing like CURL!

class CHTTPClient
{
public:
	CHTTPClient();
	~CHTTPClient();

	static bool DownloadFile(const std::string& strUrl, std::string& strResponse);
    static std::string UrlEncode(const std::string& str);

private:
	static void ProcessChunkedData(const char* data, std::string& output, std::string& chunkBuffer);
};

#endif //H_CHTTPCLEINT
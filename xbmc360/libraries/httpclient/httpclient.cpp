#include "httpclient.h"

#include <xtl.h>
#include <xonline.h>
#include <winsockx.h>
#include <stdio.h>
#include <vector>
#include "utils\log.h"

using namespace std;

// A very basic crude http client as temporary stand-in solution till we have somthing like CURL!

bool CHTTPClient::DownloadFile(const string& strUrl, string& strResponse)
{
    strResponse.clear();
    string url = strUrl;
    const string http = "http://";
    const string https = "https://";

    if (url.rfind(http, 0) == 0)
        url.erase(0, http.length());
    else if (url.rfind(https, 0) == 0)
        url.erase(0, https.length());

    auto slashPos = url.find('/');
    if (slashPos == string::npos)
	{
        strResponse = "URL must include a path";
        return false;
    }

    string host = url.substr(0, slashPos);
    string path = url.substr(slashPos);

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
        strResponse = "WSAStartup failed";
        return false;
    }

    XNetStartupParams xnsp = { sizeof(xnsp), 0 };
    xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;
    if (XNetStartup(&xnsp) != 0)
	{
        WSACleanup();
        strResponse = "XNetStartup failed";
        return false;
    }

    HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    XNDNS* pDns = nullptr;
    if (XNetDnsLookup(host.c_str(), hEvent, &pDns) != 0)
	{
        CloseHandle(hEvent);
        WSACleanup();
        strResponse = "XNetDnsLookup failed";
        return false;
    }

    while (pDns->iStatus == WSAEINPROGRESS)
	{
        WaitForSingleObject(hEvent, INFINITE);
    }
    
	CloseHandle(hEvent);

    if (pDns->iStatus != 0 || pDns->cina == 0)
	{
        XNetDnsRelease(pDns);
        WSACleanup();
        strResponse = "DNS resolution failed";
        return false;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
	{
        XNetDnsRelease(pDns);
        WSACleanup();
        strResponse = "Socket creation failed";
        return false;
    }

	BOOL bBroadcast = TRUE;
	if(setsockopt(sock, SOL_SOCKET, 0x5801, (PCSTR)&bBroadcast, sizeof(BOOL) ) != 0 )
	{
		CLog::Log(LOGERROR, "CHTTPClient: Failed to set send socket to 5801, error");
		return false;
	}

    sockaddr_in server = {};
    server.sin_family = AF_INET;
    server.sin_port = htons(80);
    server.sin_addr = pDns->aina[0];
    XNetDnsRelease(pDns);

    if (connect(sock, (SOCKADDR*)&server, sizeof(server)))
	{
        closesocket(sock);
        WSACleanup();
        strResponse = "Connect failed";
        return false;
    }

    string request = "GET " + path + " HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n\r\n";

    if (send(sock, request.c_str(), (int)request.length(), 0) == SOCKET_ERROR)
	{
        closesocket(sock);
        WSACleanup();
        strResponse = "Send failed";
        return false;
    }

    vector<char> recvBuffer(16384);
    string rawResponse;
    string chunkBuffer;
    bool chunked = false;
    string headerBuffer; // Accumulates header data across recv calls

    while (true)
	{
        int bytesReceived = recv(sock, recvBuffer.data(), recvBuffer.size() - 1, 0);
        if (bytesReceived <= 0) break;

        recvBuffer[bytesReceived] = '\0';
        string chunk(recvBuffer.data(), bytesReceived);

        // Accumulate headers until \r\n\r\n is found
        if (!chunked && headerBuffer.find("\r\n\r\n") == string::npos)
		{
            headerBuffer.append(chunk);

            size_t headerEnd = headerBuffer.find("\r\n\r\n");
            if (headerEnd != string::npos)
			{
                // Check if chunked encoding is specified in the complete headers
                if (headerBuffer.find("Transfer-Encoding: chunked") != string::npos)
				{
                    chunked = true;
                }

                // Split headers and body
                size_t bodyStart = headerEnd + 4;
                string body = headerBuffer.substr(bodyStart);
                headerBuffer.erase(bodyStart); // Keep only headers

                if (chunked)
				{
                    ProcessChunkedData(body.c_str(), rawResponse, chunkBuffer);
                }
				else
				{
                    rawResponse.append(body);
                }
            }
        }
		else
		{
            if (chunked)
                ProcessChunkedData(chunk.c_str(), rawResponse, chunkBuffer);
			else
                rawResponse.append(chunk);
        }

        if (bytesReceived == static_cast<int>(recvBuffer.size() - 1))
		{
            if (recvBuffer.size() < 1048576)
                recvBuffer.resize(recvBuffer.size() * 2);
        }
    }

    if (chunked && !chunkBuffer.empty())
        ProcessChunkedData("", rawResponse, chunkBuffer);

    closesocket(sock);
    WSACleanup();
    strResponse = move(rawResponse);

    return true;
}

void CHTTPClient::ProcessChunkedData(const char* data, string& output, string& chunkBuffer)
{
    chunkBuffer.append(data);

    while (true)
	{
        size_t chunkEnd = chunkBuffer.find("\r\n");
        if (chunkEnd == string::npos) break;

        size_t extPos = chunkBuffer.find(';');
        
		if (extPos != string::npos && extPos < chunkEnd)
            chunkEnd = extPos;

        string hexSize = chunkBuffer.substr(0, chunkEnd);
        unsigned int chunkSize;
        sscanf(hexSize.c_str(), "%x", &chunkSize);

        if (chunkSize == 0)
		{
            size_t endPos = chunkBuffer.find("\r\n\r\n");

            if (endPos != string::npos)
			{
                chunkBuffer.erase(0, endPos + 4);
            }
			else
			{
                // Ensure buffer is cleared even if no trailers
                chunkBuffer.clear();
            }
            break;
        }

        size_t dataStart = chunkEnd + 2;
        size_t requiredSize = dataStart + chunkSize + 2;

        if (chunkBuffer.length() >= requiredSize)
		{
            output.append(chunkBuffer, dataStart, chunkSize);
            chunkBuffer.erase(0, requiredSize);
        }
		else
            break;
    }
}

std::string CHTTPClient::UrlEncode(const std::string& str)
{
    // static const (C++03) lookup table
    static const char hex[] = "0123456789ABCDEF";

    std::string result;
    result.reserve(str.size() * 3);  // worst case every char is "%XY"

    for (std::string::size_type i = 0; i < str.size(); ++i)
    {
        unsigned char c = static_cast<unsigned char>(str[i]);

        // unreserved characters per RFC 3986 plus the extras you allowed
        if ((c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
             c == '-' || c == '_' ||
             c == '.' || c == '~' ||
             c == '/' || c == ':' ||
             c == '@' || c == '!' ||
             c == '$' || c == '&' ||
             c == '\''|| c == '(' ||
             c == ')' || c == '*' ||
             c == '+' || c == ',' ||
             c == ';' || c == '=')
        {
            result += static_cast<char>(c);
        }
        else
        {
            // percent‑encode
            result += '%';
            result += hex[(c >> 4) & 0xF];
            result += hex[c & 0xF];
        }
    }

    return result;
}
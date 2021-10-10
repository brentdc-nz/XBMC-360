#ifndef H_CDVDINPUTSTREAM
#define H_CDVDINPUTSTREAM

enum DVDStreamType
{
	DVDSTREAM_TYPE_NONE   = -1,
	DVDSTREAM_TYPE_FILE   = 1,
};

class CDVDInputStream
{
public:
	CDVDInputStream(DVDStreamType m_streamType);
	virtual ~CDVDInputStream();

	virtual bool Open(const char* strFileName/*, const std::string& content*/) = 0;
	virtual void Close() = 0;

	const char* GetFileName();
	bool IsStreamType(DVDStreamType type) { return m_streamType == type; }
	virtual bool IsEOF() = 0;  

protected:
	DVDStreamType m_streamType;
	char* m_strFileName;
};

#endif //H_CDVDINPUTSTREAM
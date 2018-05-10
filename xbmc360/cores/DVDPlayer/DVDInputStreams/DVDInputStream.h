#ifndef H_CDVDINPUTSTREAM
#define H_CDVDINPUTSTREAM

enum DVDStreamType
{
	DVDSTREAM_TYPE_NONE   = -1,
	DVDSTREAM_TYPE_FILE   = 1
	// TODO: More Stream types
};

class CDVDInputStream
{
public:
	CDVDInputStream();
	virtual ~CDVDInputStream();

	virtual bool Open(const char* strFile);
	virtual void Close();

	const char* GetFileName();
	bool IsStreamType(DVDStreamType type) { return m_streamType == type; }

protected:
	char* m_strFileName;
	DVDStreamType m_streamType; 
};

#endif //H_CDVDINPUTSTREAM
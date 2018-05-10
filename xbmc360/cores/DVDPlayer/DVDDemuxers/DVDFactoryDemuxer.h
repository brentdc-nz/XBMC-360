#ifndef H_CDVDFACTORYDEMUXER
#define H_CDVDFACTORYDEMUXER

class CDVDDemux;
class CDVDInputStream;

class CDVDFactoryDemuxer
{
public:
	static CDVDDemux* CreateDemuxer(CDVDInputStream* pInputStream);
};

#endif //H_CDVDFACTORYDEMUXER
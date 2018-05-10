#ifndef H_CDVDFACTORYINPUTSTREAM
#define H_CDVDFACTORYINPUTSTREAM

class CDVDInputStream;
class IDVDPlayer;

class CDVDFactoryInputStream
{
public:
	static CDVDInputStream* CreateInputStream(IDVDPlayer* pPlayer, const char* strFile);
};

#endif //H_CDVDFACTORYINPUTSTREAM
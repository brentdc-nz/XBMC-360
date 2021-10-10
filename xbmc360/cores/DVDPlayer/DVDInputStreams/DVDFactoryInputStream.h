#ifndef H_CDVDFACTORYINPUTSTREAM
#define H_CDVDFACTORYINPUTSTREAM

#include <string>

class CDVDInputStream;
class IDVDPlayer;

class CDVDFactoryInputStream
{
public:
	static CDVDInputStream* CreateInputStream(IDVDPlayer* pPlayer, const std::string& file, const std::string& content);
};

#endif //H_CDVDFACTORYINPUTSTREAM
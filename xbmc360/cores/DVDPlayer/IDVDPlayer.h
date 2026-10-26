#ifndef H_IDVDPLAYER
#define H_IDVDPLAYER

class DVDNavResult;

class IDVDPlayer
{
public:
	virtual int OnDVDNavResult(void* pData, int iMessage) = 0;
	virtual ~IDVDPlayer() { }
};

#endif //H_IDVDPLAYER
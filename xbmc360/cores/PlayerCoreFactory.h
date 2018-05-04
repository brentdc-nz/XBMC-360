#ifndef H_CPLAYERCOREFACTORY
#define H_CPLAYERCOREFACTORY

#include "IPlayer.h"

const enum EPLAYERCORES
{
	EPC_NONE,
	EPC_DVDPLAYER,
};

class CPlayerCoreFactory
{
public:
	CPlayerCoreFactory();
	virtual ~CPlayerCoreFactory();

	IPlayer* CreatePlayer(const CStdString& strCore, IPlayerCallback& callback);
	IPlayer* CreatePlayer(const EPLAYERCORES eCore, IPlayerCallback& callback);
	EPLAYERCORES GetPlayerCore(const CStdString& strCore);
};

#endif //H_CPLAYERCOREFACTORY
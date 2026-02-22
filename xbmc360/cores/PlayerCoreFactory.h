#ifndef H_CPLAYERCOREFACTORY
#define H_CPLAYERCOREFACTORY

#include "IPlayer.h"

typedef unsigned int PLAYERCOREID;

const enum EPLAYERCORES
{
	EPC_NONE,
	EPC_DVDPLAYER,
	EPC_PAPLAYER
};

class CFileItem;

class CPlayerCoreFactory
{
public:
	CPlayerCoreFactory();
	virtual ~CPlayerCoreFactory();

	static IPlayer* CreatePlayer(const CStdString& strCore, IPlayerCallback& callback);
	static IPlayer* CreatePlayer(const PLAYERCOREID eCore, IPlayerCallback& callback);

	static PLAYERCOREID GetDefaultPlayer(const CFileItem& item);
	static PLAYERCOREID GetPlayerCore(const CStdString& strCore);
	static CStdString GetPlayerName(const PLAYERCOREID eCore);
};

#endif //H_CPLAYERCOREFACTORY
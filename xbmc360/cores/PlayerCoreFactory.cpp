#include "PlayerCoreFactory.h"
#include "DVDPlayer\DVDPlayer.h"

CPlayerCoreFactory::CPlayerCoreFactory()
{
}

CPlayerCoreFactory::~CPlayerCoreFactory()
{
}

IPlayer* CPlayerCoreFactory::CreatePlayer(const CStdString& strCore, IPlayerCallback& callback)
{ 
	return CreatePlayer( GetPlayerCore(strCore), callback ); 
}

IPlayer* CPlayerCoreFactory::CreatePlayer(const EPLAYERCORES eCore, IPlayerCallback& callback)
{
	switch(eCore)
	{
		// We only have DVDPlayer at the momment..
		case EPC_DVDPLAYER: return new CDVDPlayer(callback);
	}

	return NULL;
}

EPLAYERCORES CPlayerCoreFactory::GetPlayerCore(const CStdString& strCore)
{
	CStdString strCoreLower = strCore;
	strCoreLower.ToLower();

	if (strCoreLower == "dvdplayer") return EPC_DVDPLAYER;

//	return EPC_NONE;
	return EPC_DVDPLAYER; // Default atm
}
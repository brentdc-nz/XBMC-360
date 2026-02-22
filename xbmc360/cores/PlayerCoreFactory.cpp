#include "PlayerCoreFactory.h"
#include "DVDPlayer\DVDPlayer.h"
#include "PAPlayer\PAPlayer.h"
#include "FileItem.h"
#include "utils\Log.h"

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

IPlayer* CPlayerCoreFactory::CreatePlayer(const PLAYERCOREID eCore, IPlayerCallback& callback)
{
	switch(eCore)
	{
		case EPC_DVDPLAYER:
			CLog::Log(LOGDEBUG, "CPlayerCoreFactory::CreatePlayer: creating DVDPlayer");
			return new CDVDPlayer(callback);

		case EPC_PAPLAYER:
			CLog::Log(LOGDEBUG, "CPlayerCoreFactory::CreatePlayer: creating PAPlayer");
			return new PAPlayer(callback);
	}

	return NULL;
}

PLAYERCOREID CPlayerCoreFactory::GetDefaultPlayer(const CFileItem& item)
{
	// If item is audio and PAPlayer can handle the type, use PAPlayer
	if (item.IsAudio())
	{
		CStdString strExtension;
		CStdString strPath = item.GetPath();

		// Extract extension from path
		int iPos = strPath.ReverseFind('.');

		if (iPos > 0)
		{
			strExtension = strPath.Right(strPath.GetLength() - iPos - 1);
			strExtension.ToLower();
		}

		if (PAPlayer::HandlesType(strExtension))
		{
			CLog::Log(LOGDEBUG, "CPlayerCoreFactory::GetDefaultPlayer: PAPlayer handles extension '%s'", strExtension.c_str());
			return EPC_PAPLAYER;
		}

		// Audio file but PAPlayer can't handle it - fall through to DVDPlayer
		CLog::Log(LOGDEBUG, "CPlayerCoreFactory::GetDefaultPlayer: PAPlayer doesn't handle '%s', falling back to DVDPlayer", strExtension.c_str());
	}

	// Default for video or anything else: DVDPlayer
	CLog::Log(LOGDEBUG, "CPlayerCoreFactory::GetDefaultPlayer: using DVDPlayer");
	return EPC_DVDPLAYER;
}

PLAYERCOREID CPlayerCoreFactory::GetPlayerCore(const CStdString& strCore)
{
	CStdString strCoreLower = strCore;
	strCoreLower.ToLower();

	if (strCoreLower == "dvdplayer") return EPC_DVDPLAYER;
	if (strCoreLower == "paplayer") return EPC_PAPLAYER;

	return EPC_NONE;
}

CStdString CPlayerCoreFactory::GetPlayerName(const PLAYERCOREID eCore)
{
	switch(eCore)
	{
		case EPC_DVDPLAYER: return "DVDPlayer";
		case EPC_PAPLAYER:  return "PAPlayer";
		default: return "Unknown";
	}
}
#ifndef  IPLAYER_H
#define  IPLAYER_H

#include <string>
#include "..\FileItem.h"
#include "..\utils\StdString.h"
#include "..\guilib\key.h"

class IPlayerCallback
{
public:
	virtual void OnPlayBackEnded() = 0;
	virtual void OnPlayBackStarted() = 0;
	virtual void OnPlayBackStopped() = 0;
	virtual void OnPlayBackPaused() {};
	virtual void OnPlayBackResumed() {};
	virtual void OnQueueNextItem() = 0;
	virtual void OnPlayBackSeek(int iTime, int seekOffset) {};
	virtual void OnPlayBackSpeedChanged(int iSpeed) {};
};

class CPlayerOptions
{
public:
	CPlayerOptions()
	{
		starttime = 0LL;
		identify = false;
		fullscreen = false;
		video_only = false;
	}

	double  starttime;  // Start time in seconds
	bool    identify;   // Identify mode, used for checking format and length of a file
	CStdString state;   // Potential playerstate to restore to
	bool    fullscreen; // Player is allowed to switch to fullscreen
	bool    video_only; // Player is not allowed to play audio streams, video streams only
};

class IPlayer
{
public:
	IPlayer(IPlayerCallback& callback): m_callback(callback){};
	virtual ~IPlayer(){};

	virtual bool OpenFile(const CFileItem& file, const CPlayerOptions& options){ return false;}
	virtual bool CloseFile(){ return true;};
	virtual void SeekTime(__int64 iTime = 0){};
	virtual void Seek(bool bPlus, bool bLargeStep) = 0;
	virtual __int64 GetTime(){ return 0;};
	virtual int GetTotalTime(){ return 0;};
	virtual float GetPercentage(){ return 0;}
	virtual void GetVideoInfo(CStdString& strVideoInfo) = 0;
	virtual void GetAudioInfo(CStdString& strAudioInfo) = 0;
	virtual void GetGeneralInfo(CStdString& strGeneralInfo) = 0;
	virtual bool IsPlaying() const { return false;} ;
	virtual void Pause() = 0;
	virtual bool IsPaused() const = 0;
	virtual bool HasVideo() const = 0;
	virtual bool HasAudio() const = 0;
	virtual bool CanSeek() {return true;}
	virtual void SetVolume(long nVolume){}
	virtual bool OnAction(const CAction &action){ return false; };

	// Returns true if not playback (paused or stopped beeing filled)
	virtual bool IsCaching() const {return false;};

protected:
	IPlayerCallback& m_callback;
};

#endif //IPLAYER_H
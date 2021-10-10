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
	virtual void OnQueueNextItem() = 0;
};

class IPlayer
{
public:
  IPlayer(IPlayerCallback& callback): m_callback(callback){};
  virtual ~IPlayer(){};

	virtual bool OpenFile(const CFileItem& file) { return false; };
	virtual bool CloseFile(){ return true;};
	virtual void SeekTime(__int64 iTime = 0){};
	virtual void Seek(bool bPlus, bool bLargeStep) = 0;
	virtual __int64 GetTime(){ return 0;};
	virtual int GetTotalTime(){ return 0;};
	virtual void GetVideoInfo(CStdString& strVideoInfo) = 0;
	virtual void GetAudioInfo(CStdString& strAudioInfo) = 0;
	virtual void GetGeneralInfo(CStdString& strGeneralInfo) = 0;
	virtual bool IsPlaying() const { return false;} ;
	virtual void Pause() = 0;
	virtual bool IsPaused() const = 0;
	virtual bool HasVideo() = 0;
	virtual bool HasAudio() = 0;
	virtual bool OnAction(const CAction &action) { return false; };

protected:
	IPlayerCallback& m_callback;
};

#endif //IPLAYER_H
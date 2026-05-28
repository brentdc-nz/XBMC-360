#ifndef H_CAPPLICATIONMESSENGER
#define H_CAPPLICATIONMESSENGER

#include "utils\StdString.h"
#include "utils\Stdafx.h"
#include "utils\SingleLock.h"
#include <queue>

class CFileItem;

// Defines here
#define TMSG_DIALOG_DOMODAL       100
#define TMSG_EXECUTE_BUILT_IN     103
#define TMSG_SHUTDOWN             300
#define TMSG_POWERDOWN            301
#define TMSG_REBOOT               306
#define TMSG_NETWORKMESSAGE       500
#define TMSG_SWITCHTOFULLSCREEN   308

#define TMSG_MEDIA_PLAY           200
#define TMSG_MEDIA_STOP           201
#define TMSG_MEDIA_PAUSE          202
#define TMSG_PLAYLISTPLAYER_PLAY  210
#define TMSG_PLAYLISTPLAYER_NEXT  211
#define TMSG_PLAYLISTPLAYER_PREV  212

typedef struct
{
	DWORD dwMessage;
	DWORD dwParam1;
	DWORD dwParam2;
	CStdString strParam;
	std::vector<CStdString> params;
	HANDLE hWaitEvent;
	LPVOID lpVoid;
}
ThreadMessage;

class CApplicationMessenger
{
public:
	~CApplicationMessenger();

	void Cleanup();
	// If a message has to be send to the gui, use MSG_TYPE_WINDOW instead
	void SendMessage(ThreadMessage& msg, bool wait = false);
	void ProcessMessages(); // Only call from main thread
	void ProcessWindowMessages();

	void Shutdown();
	void Reboot();
	void SwitchToFullscreen();

	void NetworkMessage(DWORD dwMessage, DWORD dwParam = 0);

	void MediaPlay(std::string filename);
	void MediaPlay(const CFileItem &item);
	void MediaStop();
	void MediaPause();
	void PlayListPlayerPlay(int iSong = -1);
	void PlayListPlayerNext();
	void PlayListPlayerPrevious();

	void ExecBuiltIn(const CStdString &command);

private:
	void ProcessMessage(ThreadMessage *pMsg);

	CCriticalSection m_critSection;
	std::queue<ThreadMessage*> m_vecMessages;
	std::queue<ThreadMessage*> m_vecWindowMessages;
};

#endif //H_CAPPLICATIONMESSENGER
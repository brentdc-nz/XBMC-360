#ifndef H_CGUIAUDIOMANAGER
#define H_CGUIAUDIOMANAGER

#include "..\utils\SingleLock.h"
#include "..\utils\StdString.h"

#include <map>
#include <vector>

// Forward definitions
class CAction;
class CGUISound;

class CGUIAudioManager
{
public:
	CGUIAudioManager();
	~CGUIAudioManager();

	void Initialize();
	bool Load();
	void PlayActionSound(const CAction& action);
	void Cleanup();

private:
	typedef std::map<int, CStdString> actionSoundMap;
	actionSoundMap      m_actionSoundMap;
	std::vector<CGUISound*>	m_vecActionSounds;

	CCriticalSection    m_cs;
};

extern CGUIAudioManager g_audioManager;

#endif //H_CGUIAUDIOMANAGER
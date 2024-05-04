#ifndef H_CGUIAUDIOMANAGER
#define H_CGUIAUDIOMANAGER

#include "utils\SingleLock.h"
#include "utils\StdString.h"
#include "tinyxml\tinyxml.h"

// Forward definitions
class CAction;
class CGUISound;

enum WINDOW_SOUND { SOUND_INIT = 0, SOUND_DEINIT };

class CGUIAudioManager
{
	class CWindowSounds
	{
		public:
		CStdString strInitFile;
		CStdString strDeInitFile;
	};

public:
	CGUIAudioManager();
	virtual ~CGUIAudioManager();

	void Initialize();
	void DeInitialize();

	bool Load();

	void PlayActionSound(const CAction& action);
	void PlayWindowSound(int id, WINDOW_SOUND event);
	void PlayPythonSound(const CStdString& strFileName);

	void FreeUnused();

	void Enable(bool bEnable);
	void SetVolume(int iLevel);
	void Stop();

private:
	bool LoadWindowSound(TiXmlNode* pWindowNode, const CStdString& strIdentifier, CStdString& strFile);

	typedef std::map<int, CStdString> actionSoundMap;
	typedef std::map<int, CWindowSounds> windowSoundMap;

	typedef std::map<CStdString, CGUISound*> pythonSoundsMap;
	typedef std::map<int, CGUISound*> windowSoundsMap;

	actionSoundMap m_actionSoundMap;
	windowSoundMap m_windowSoundMap;

	int m_iVolume;
	CGUISound* m_actionSound;
	windowSoundsMap m_windowSounds;
	pythonSoundsMap m_pythonSounds;

	CStdString m_strMediaDir;
	bool m_bEnabled;

	CCriticalSection m_cs;
};

extern CGUIAudioManager g_audioManager;

#endif //H_CGUIAUDIOMANAGER
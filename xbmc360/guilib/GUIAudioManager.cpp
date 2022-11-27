#include "GUIAudioManager.h"
#include "GUISound.h"
#include "..\utils\Log.h"
#include "GraphicContext.h"
#include "..\ButtonTranslator.h"
#include "..\Application.h"

CGUIAudioManager g_audioManager;

CGUIAudioManager::CGUIAudioManager()
{
}

CGUIAudioManager::~CGUIAudioManager()
{
}

// Load the config file (sounds.xml) for nav sounds
// Can be located in a folder "sounds" in the skin or from a
// subfolder of the folder "sounds" in the root directory of XBMC
bool CGUIAudioManager::Load()
{
	m_actionSoundMap.clear();
	Cleanup(); // Cleanup incase a new skin is loaded

//	if(g_guiSettings.GetString("lookandfeel.soundskin")=="OFF") // TODO
//		return true;

	CStdString strSoundsXml = g_graphicsContext.GetMediaDir() + "\\sounds\\sounds.xml";

	// Load our xml file
	TiXmlDocument xmlDoc;

	CLog::Log(LOGINFO, "Loading %s", strSoundsXml.c_str());

	// Load the config file
	if (!xmlDoc.LoadFile(strSoundsXml))
	{
		CLog::Log(LOGNOTICE, "%s, Line %d\n%s", strSoundsXml.c_str(), xmlDoc.ErrorRow(), xmlDoc.ErrorDesc());
		return false;
	}

	TiXmlElement* pRoot = xmlDoc.RootElement();
	CStdString strValue = pRoot->Value();

	if (strValue != "sounds")
	{
		CLog::Log(LOGNOTICE, "%s Doesn't contain <sounds>", strSoundsXml.c_str());
		return false;
	}

	// Load sounds for actions
	TiXmlElement* pActions = pRoot->FirstChildElement("actions");

	if (pActions)
	{
		TiXmlNode* pAction = pActions->FirstChild("action");

		while (pAction)
		{
			TiXmlNode* pIdNode = pAction->FirstChild("name");
			int id = 0; // Action identity
			
			if (pIdNode && pIdNode->FirstChild())
			{
				g_buttonTranslator.TranslateActionString(pIdNode->FirstChild()->Value(), id);
			}

			TiXmlNode* pFileNode = pAction->FirstChild("file");
			CStdString strFile;

			if (pFileNode && pFileNode->FirstChild())
				strFile+=pFileNode->FirstChild()->Value();

			if (id > 0 && !strFile.IsEmpty())
				m_actionSoundMap.insert(pair<int, CStdString>(id, strFile));

			pAction = pAction->NextSibling();
		}
	}

	// Load window specific sounds
	TiXmlElement* pWindows = pRoot->FirstChildElement("windows");

	if (pWindows)
	{
		TiXmlNode* pWindow = pWindows->FirstChild("window");

		while (pWindow)
		{
			int id = 0;

			TiXmlNode* pIdNode = pWindow->FirstChild("name");
			
			if (pIdNode)
			{
				if (pIdNode->FirstChild())
					id = g_buttonTranslator.TranslateWindowString(pIdNode->FirstChild()->Value());
			}

			CWindowSounds sounds;
			LoadWindowSound(pWindow, "activate", sounds.strInitFile);
			LoadWindowSound(pWindow, "deactivate", sounds.strDeInitFile);

			if(id > 0)
				m_windowSoundMap.insert(pair<int, CWindowSounds>(id, sounds));

			pWindow = pWindow->NextSibling();
		}
	}

	return true;
}

// Load a window node of the config file (sounds.xml)
bool CGUIAudioManager::LoadWindowSound(TiXmlNode* pWindowNode, const CStdString& strIdentifier, CStdString& strFile)
{
	if (!pWindowNode)
		return false;

	TiXmlNode* pFileNode = pWindowNode->FirstChild(strIdentifier);
	
	if (pFileNode && pFileNode->FirstChild())
	{
		strFile = pFileNode->FirstChild()->Value();
		return true;
	}

	return false;
}

// Play a sound associated with a CAction
void CGUIAudioManager::PlayActionSound(const CAction& action)
{
	CSingleLock lock(m_cs);

	// Disable navigation sounds while playing
//	if (g_guiSettings.GetString("lookandfeel.navsoundplaying")=="OFF") // TODO
	{
		if(g_application.IsPlaying() && !g_application.IsPaused())
			return;
	}

	actionSoundMap::iterator it=m_actionSoundMap.find(action.GetID());
	
	if (it == m_actionSoundMap.end())
		return;

	CGUISound* pActionSound = NULL;

	for(int i = 0; i < (int)m_vecActionSounds.size(); i++)
	{
		pActionSound = m_vecActionSounds[i];

		if(pActionSound->GetFileName() == it->second)
		{
			pActionSound->Play();
			return;
		}
	}

	pActionSound = new CGUISound();

	if (!pActionSound->Load(it->second))
	{
		delete pActionSound;
		pActionSound = NULL;
		return;
	}

	m_vecActionSounds.push_back(pActionSound);

	pActionSound->Play();
}

// Play a sound associated with a window and its event
// Events: SOUND_INIT, SOUND_DEINIT
void CGUIAudioManager::PlayWindowSound(int id, WINDOW_SOUND event)
{
	CSingleLock lock(m_cs);

	windowSoundMap::iterator it= m_windowSoundMap.find(id);

	if(it == m_windowSoundMap.end())
		return;

	CWindowSounds sounds=it->second;
	CStdString strFile;

	switch(event)
	{
	case SOUND_INIT:
		strFile = sounds.strInitFile;
		break;
	case SOUND_DEINIT:
		strFile = sounds.strDeInitFile;
		break;
	}

	if (strFile.IsEmpty())
		return;

	CGUISound* pWindowSound = NULL;

	for(int i = 0; i < (int)m_vecWindowSounds.size(); i++)
	{
		pWindowSound = m_vecWindowSounds[i];

		if(pWindowSound->GetFileName() == strFile)
		{
			pWindowSound->Play();
			return;
		}
	}

	pWindowSound = new CGUISound();

	if (!pWindowSound->Load(strFile))
	{
		delete pWindowSound;
		pWindowSound = NULL;
		return;
	}

	m_vecWindowSounds.push_back(pWindowSound);

	pWindowSound->Play();
}


void CGUIAudioManager::Cleanup()
{
	CSingleLock lock(m_cs);

	// Delete action sounds
	CGUISound* pActionSound = NULL;

	for(int i = 0; i < (int)m_vecActionSounds.size(); i++)
	{
		pActionSound = m_vecActionSounds[i];
		
		if(pActionSound)
		{
			pActionSound->FreeBuffer();
			delete pActionSound;
			pActionSound = NULL;
		}
	}

	m_vecActionSounds.clear();

	// Delete window sounds
	CGUISound* pWindowSound = NULL;

	for(int i = 0; i < (int)m_vecWindowSounds.size(); i++)
	{
		pWindowSound = m_vecWindowSounds[i];
		
		if(pWindowSound)
		{
			pWindowSound->FreeBuffer();
			delete pWindowSound;
			pWindowSound = NULL;
		}
	}

	m_vecWindowSounds.clear();
}
#ifndef H_CSETTINGS
#define H_CSETTINGS

#include "utils\StdString.h"
#include "guilib\tinyxml\tinyxml.h"
#include "MediaManager.h"
#include "VideoSettings.h"

#define DEFAULT_SKIN "Project Mayhem III"
#define SETTINGS_FILE "D:\\settings.xml"
#define SOURCES_FILE "D:\\sources.xml" //TODO: GetUserDataFolder()

class CSettings
{
public:
	CSettings();
	virtual ~CSettings();

	bool LoadSettings(const CStdString& strSettingsFile);
	bool Load();
	bool SaveSettings(const CStdString& strSettingsFile) const;
	void Save() const;

	bool AddShare(const CStdString &type, const CMediaSource &share);
	VECSOURCES *GetSourcesFromType(const CStdString &type);
	bool DeleteSource(const CStdString &strType, const CStdString strName, const CStdString strPath, bool virtualSource = false);

	CStdString GetVideoExtensions() { return m_strVideoExtensions; };
	CStdString GetAudioExtensions() { return m_strAudioExtensions; };
	CStdString GetPictureExtensions() { return m_strPictureExtensions; };

	struct stSettings
	{
		public:
		CVideoSettings m_currentVideoSettings;
	};

protected:
	void GetSources(const TiXmlElement* pRootElement, const CStdString& strTagName, VECSOURCES& items);
	bool GetSource(const CStdString &category, const TiXmlNode *source, CMediaSource &share);
	bool SetSources(TiXmlNode *root, const char *section, const VECSOURCES &shares);
	bool SaveSources();

	void GetInteger(const TiXmlElement* pRootElement, const CStdString& strTagName, int& iValue, const int iDefault, const int iMin, const int iMax);

	VECSOURCES m_vecProgramSources;
	VECSOURCES m_vecVideoSources;
	VECSOURCES m_vecMusicSources;
	VECSOURCES m_vecPictureSources;

	CStdString m_strVideoExtensions;
	CStdString m_strAudioExtensions;
	CStdString m_strPictureExtensions;
};

extern CSettings g_settings;
extern struct CSettings::stSettings g_stSettings;

#endif //H_CSETTINGS
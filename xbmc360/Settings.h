#ifndef H_CSETTINGS
#define H_CSETTINGS

#include "utils\StdString.h"
#include "guilib\tinyxml\tinyxml.h"
#include "MediaSource.h"
#include "VideoSettings.h"
#include "guilib\GraphicContext.h"

#define DEFAULT_SKIN "Project Mayhem III"
#define SETTINGS_FILE "D:\\settings.xml"
#define SOURCES_FILE "D:\\sources.xml" //TODO: GetUserDataFolder()

class CSettings
{
public:
	CSettings();
	virtual ~CSettings();

	void Initialize();
	bool LoadSettings(const CStdString& strSettingsFile);
	bool Load();
	bool SaveSettings(const CStdString& strSettingsFile) const;
	void Save() const;

	bool UpdateShare(const CStdString &type, const CStdString oldName, const CMediaSource &share);
	bool AddShare(const CStdString &type, const CMediaSource &share);
	VECSOURCES *GetSourcesFromType(const CStdString &type);
	bool DeleteSource(const CStdString &strType, const CStdString strName, const CStdString strPath, bool virtualSource = false);

	CStdString GetVideoExtensions() { return m_strVideoExtensions; };
	CStdString GetAudioExtensions() { return m_strAudioExtensions; };
	CStdString GetPictureExtensions() { return m_strPictureExtensions; };

	CStdString m_logFolder;
	int m_iSystemTimeTotalUp; // Uptime in minutes!

	CVideoSettings m_currentVideoSettings;

	RESOLUTION_INFO m_ResInfo[10];

	VECSOURCES m_programSources;
	VECSOURCES m_pictureSources;
	VECSOURCES m_fileSources;
	VECSOURCES m_musicSources;
	VECSOURCES m_videoSources;

protected:
	void GetSources(const TiXmlElement* pRootElement, const CStdString& strTagName, VECSOURCES& items);
	bool GetSource(const CStdString &category, const TiXmlNode *source, CMediaSource &share);
	bool SetSources(TiXmlNode *root, const char *section, const VECSOURCES &shares);
	bool SaveSources();

	void GetInteger(const TiXmlElement* pRootElement, const CStdString& strTagName, int& iValue, const int iDefault, const int iMin, const int iMax);

	CStdString m_strVideoExtensions;
	CStdString m_strAudioExtensions;
	CStdString m_strPictureExtensions;
};

extern class CSettings g_settings;

#endif //H_CSETTINGS
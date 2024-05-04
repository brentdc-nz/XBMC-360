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

#define VOLUME_MINIMUM -6000  // -60dB
#define VOLUME_MAXIMUM 0      // 0dB
#define VOLUME_DRC_MINIMUM 0    // 0dB
#define VOLUME_DRC_MAXIMUM 3000 // 30dB

class CSkinString
{
public:
	CStdString name;
	CStdString value;
};

class CSkinBool
{
public:
	CStdString name;
	bool value;
};

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

	void Clear();

	bool UpdateShare(const CStdString &type, const CStdString oldName, const CMediaSource &share);
	bool AddShare(const CStdString &type, const CMediaSource &share);
	VECSOURCES *GetSourcesFromType(const CStdString &type);
	bool DeleteSource(const CStdString &strType, const CStdString strName, const CStdString strPath, bool virtualSource = false);

	int TranslateSkinString(const CStdString &setting);
	const CStdString &GetSkinString(int setting) const;
	void SetSkinString(int setting, const CStdString &label);

	int TranslateSkinBool(const CStdString &setting);
	bool GetSkinBool(int setting) const;
	void SetSkinBool(int setting, bool set);

	void ResetSkinSetting(const CStdString &setting);
	void ResetSkinSettings();

	CStdString GetVideoExtensions() { return m_strVideoExtensions; };
	CStdString GetAudioExtensions() { return m_strAudioExtensions; };
	CStdString GetPictureExtensions() { return m_strPictureExtensions; };

	CStdString m_logFolder;
	int m_iSystemTimeTotalUp; // Uptime in minutes!

	CVideoSettings m_currentVideoSettings;

	RESOLUTION_INFO m_ResInfo[10];

	int m_nVolumeLevel; // Measured in milliBels -60dB -> 0dB range.
	int m_dynamicRangeCompressionLevel; // Measured in milliBels  0dB -> 30dB range.
	int m_iPreMuteVolumeLevel; // Save the m_nVolumeLevel for proper restore
	bool m_bMute;

	VECSOURCES m_programSources;
	VECSOURCES m_pictureSources;
	VECSOURCES m_fileSources;
	VECSOURCES m_musicSources;
	VECSOURCES m_videoSources;

protected:
	std::map<int, CSkinString> m_skinStrings;
	std::map<int, CSkinBool> m_skinBools;

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
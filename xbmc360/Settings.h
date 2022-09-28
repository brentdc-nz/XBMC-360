#ifndef H_CSETTINGS
#define H_CSETTINGS

#include "utils\StdString.h"
#include "guilib\tinyxml\tinyxml.h"
#include "MediaManager.h"
#include "VideoSettings.h"
#include "Profile.h"
#include "filesystem/Directory.h"
#include "guilib/LocalizeStrings.h"

#define DEFAULT_SKIN "Project Mayhem III"

class CSettings
{
public:
	CSettings();
	virtual ~CSettings();

	void Initialize();
	void LoadExtensions();
	bool LoadSettings(const CStdString& strSettingsFile);
	bool Load();
	bool SaveSettings(const CStdString& strSettingsFile) const;
	void Save() const;
	void ClearSources();
	bool AddShare(const CStdString &type, const CMediaSource &share);
	VECSOURCES *GetSourcesFromType(const CStdString &type);
	bool DeleteSource(const CStdString &strType, const CStdString strName, const CStdString strPath, bool virtualSource = false);

	CStdString GetVideoExtensions() { return m_strVideoExtensions; };
	CStdString GetAudioExtensions() { return m_strAudioExtensions; };
	CStdString GetPictureExtensions() { return m_strPictureExtensions; };

	//Profilers...
	bool LoadProfile(unsigned int index);
	bool DeleteProfile(unsigned int index);
	void DeleteAllProfiles();
	void CreateProfileFolders();
	const CProfile &GetMasterProfile() const;
	const CProfile &GetCurrentProfile() const;
	const CProfile *GetProfile(unsigned int index) const;
    CProfile *GetProfile(unsigned int index);
	int GetProfileIndex(const CStdString &name) const;
	unsigned int GetNumProfiles() const;
	void AddProfile(const CProfile &profile);
	bool UsingLoginScreen() const { return m_usingLoginScreen; };
	void ToggleLoginScreen() { m_usingLoginScreen = !m_usingLoginScreen; };
    bool IsMasterUser() const { return 0 == m_currentProfile; };
	void UpdateCurrentProfileDate();
    bool SaveProfiles(const CStdString& profilesFile) const;
	void LoadMasterForLogin();

	unsigned int GetLastUsedProfileIndex() const { return m_lastUsedProfile; };
	unsigned int GetCurrentProfileIndex() const { return m_currentProfile; };

	//uses HasSlashAtEnd to determine if a directory or file was meant
	CStdString GetUserDataItem(const CStdString& strFile) const;
#ifdef WIP
	CStdString GetDatabaseFolder() const;
	CStdString GetCDDBFolder() const;
	CStdString GetThumbnailsFolder() const;
	CStdString GetMusicThumbFolder() const;
	CStdString GetLastFMThumbFolder() const;
	CStdString GetMusicArtistThumbFolder() const;
	CStdString GetVideoThumbFolder() const;
	CStdString GetBookmarksThumbFolder() const;
	CStdString GetPicturesThumbFolder() const;
	CStdString GetProgramsThumbFolder() const;
	CStdString GetGameSaveThumbFolder() const;
	CStdString GetProfilesThumbFolder() const;
	CStdString GetScriptsFolder() const;
	CStdString GetVideoFanartFolder() const;
	CStdString GetMusicFanartFolder() const;
	CStdString GetFFmpegDllFolder() const;
	CStdString GetPlayerName(const int& player) const;
	CStdString GetDefaultVideoPlayerName() const;
	CStdString GetDefaultAudioPlayerName() const;
	CStdString GetAvpackSettingsFile() const;
#endif
    CStdString GetSkinFolder() const;
	CStdString GetSkinFolder(const CStdString& skinName) const;	
	CStdString GetSettingsFile() const;
	CStdString GetProfileUserDataFolder() const;
	CStdString GetUserDataFolder() const;
	CStdString GetSourcesFile() const;

	int m_iSystemTimeTotalUp; // Uptime in minutes!

	CVideoSettings m_currentVideoSettings;
	CStdString m_logFolder;
private:
    unsigned int m_lastUsedProfile;
    unsigned int m_currentProfile;
    std::vector<CProfile> m_vecProfiles;
    bool m_usingLoginScreen;
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

extern class CSettings g_settings;

#endif //H_CSETTINGS
#ifndef CGUIINFOMANAGER_H
#define CGUIINFOMANAGER_H

#include "guilib\GUIWindow.h"
#include "utils\StdString.h"
#include "utils\Stdafx.h"
#include "utils\TimeUtils.h"
#include "guilib\info\SkinVariable.h"
#include "guilib\info\InfoBool.h"

#include <list>
#include <map>

namespace MUSIC_INFO
{
	class CMusicInfoTag;
}
class CVideoInfoTag;

namespace INFO
{
	class InfoBool;
}

// Conditions for window retrieval
#define WINDOW_CONDITION_HAS_LIST_ITEMS  1
#define WINDOW_CONDITION_IS_MEDIA_WINDOW 2

#define KB  (1024)          // 1 KiloByte (1KB)   1024 Byte (2^10 Byte)
#define MB  (1024*KB)       // 1 MegaByte (1MB)   1024 KB (2^10 KB)
#define GB  (1024*MB)       // 1 GigaByte (1GB)   1024 MB (2^10 MB)
#define TB  (1024*GB)       // 1 TerraByte (1TB)  1024 GB (2^10 GB)

#define OPERATOR_NOT  3
#define OPERATOR_AND  2
#define OPERATOR_OR   1

#define PLAYER_HAS_MEDIA            1
#define PLAYER_HAS_AUDIO            2
#define PLAYER_HAS_VIDEO            3
#define PLAYER_PAUSED               5
#define PLAYER_REWINDING            6
#define PLAYER_REWINDING_2x         7
#define PLAYER_REWINDING_4x         8
#define PLAYER_REWINDING_8x         9
#define PLAYER_REWINDING_16x        10
#define PLAYER_REWINDING_32x        11
#define PLAYER_FORWARDING           12
#define PLAYER_FORWARDING_2x        13
#define PLAYER_FORWARDING_4x        14
#define PLAYER_FORWARDING_8x        15
#define PLAYER_FORWARDING_16x       16
#define PLAYER_FORWARDING_32x       17
#define PLAYER_DISPLAY_AFTER_SEEK   21
#define PLAYER_PROGRESS             22
#define PLAYER_SEEKBAR              23
#define PLAYER_SEEKTIME             24
#define PLAYER_SEEKING              25
#define PLAYER_TIME                 27
#define PLAYER_TIME_REMAINING       28
#define PLAYER_DURATION             29
#define PLAYER_SHOWCODEC            30
#define PLAYER_SHOWINFO             31
#define PLAYER_VOLUME               32
#define PLAYER_MUTED                33
#define PLAYER_TIME_SPEED           37
#define PLAYER_SEEKOFFSET           47

#define WEATHER_CONDITIONS          100
#define WEATHER_TEMPERATURE         101
#define WEATHER_LOCATION            102
#define WEATHER_IS_FETCHED          103
#define WEATHER_FANART_CODE         104
#define WEATHER_PLUGIN              105

#define SYSTEM_PROGRESS_BAR         107
#define SYSTEM_TIME                 110
#define SYSTEM_DATE                 111
#define SYSTEM_CPU_TEMPERATURE      112
#define SYSTEM_GPU_TEMPERATURE      113
#define SYSTEM_FPS                  123
#define SYSTEM_ALWAYS_TRUE          125   // Useful for <visible fade="10" start="hidden">true</visible>, to fade in a control
#define SYSTEM_ALWAYS_FALSE         126   // Used for <visible fade="10">false</visible>, to fade out a control (i.e not particularly useful!)

#define SYSTEM_INTERNET_STATE       159

#define NETWORK_IP_ADDRESS          190
#define NETWORK_MAC_ADDRESS         191
#define NETWORK_IS_DHCP             192
#define NETWORK_LINK_STATE          193
#define NETWORK_SUBNET_ADDRESS      194
#define NETWORK_GATEWAY_ADDRESS     195
#define NETWORK_DNS1_ADDRESS        196
#define NETWORK_DNS2_ADDRESS        197
#define NETWORK_DHCP_ADDRESS        198

#define MUSICPLAYER_TITLE           200
#define MUSICPLAYER_ALBUM           201
#define MUSICPLAYER_ARTIST          202
#define MUSICPLAYER_GENRE           203
#define MUSICPLAYER_YEAR            204
#define MUSICPLAYER_DURATION        205
#define MUSICPLAYER_TRACK_NUMBER    208
#define MUSICPLAYER_COVER           210
#define MUSICPLAYER_BITRATE         211
#define MUSICPLAYER_PLAYLISTLEN     212
#define MUSICPLAYER_PLAYLISTPOS     213
#define MUSICPLAYER_CHANNELS        214
#define MUSICPLAYER_BITSPERSAMPLE   215
#define MUSICPLAYER_SAMPLERATE      216
#define MUSICPLAYER_CODEC           217
#define MUSICPLAYER_DISC_NUMBER     218
#define MUSICPLAYER_RATING          219
#define MUSICPLAYER_COMMENT         220
#define MUSICPLAYER_LYRICS          221
#define MUSICPLAYER_HASPREVIOUS     222
#define MUSICPLAYER_HASNEXT         223
#define MUSICPLAYER_EXISTS          224
#define MUSICPLAYER_PLAYLISTPLAYING 225
#define MUSICPLAYER_ALBUM_ARTIST    226
#define MUSICPLAYER_PLAYCOUNT       227
#define MUSICPLAYER_LASTPLAYED      228
#define VIDEOPLAYER_TITLE             250
#define VIDEOPLAYER_GENRE             251
#define VIDEOPLAYER_DIRECTOR          252
#define VIDEOPLAYER_YEAR              253
#define VIDEOPLAYER_HASSUBTITLES      254
#define VIDEOPLAYER_SUBTITLESENABLED  255
#define VIDEOPLAYER_COVER             258
#define VIDEOPLAYER_USING_OVERLAYS    259
#define VIDEOPLAYER_ISFULLSCREEN      260
#define VIDEOPLAYER_HASMENU           261
#define VIDEOPLAYER_PLAYLISTLEN       262
#define VIDEOPLAYER_PLAYLISTPOS       263
#define VIDEOPLAYER_EVENT             264
#define VIDEOPLAYER_ORIGINALTITLE     265
#define VIDEOPLAYER_PLOT              266
#define VIDEOPLAYER_PLOT_OUTLINE      267
#define VIDEOPLAYER_EPISODE           268
#define VIDEOPLAYER_SEASON            269
#define VIDEOPLAYER_RATING            270
#define VIDEOPLAYER_TVSHOW            271
#define VIDEOPLAYER_PREMIERED         272
#define VIDEOPLAYER_CONTENT           273
#define VIDEOPLAYER_STUDIO            274
#define VIDEOPLAYER_MPAA              275
#define VIDEOPLAYER_CAST              276
#define VIDEOPLAYER_CAST_AND_ROLE     277
#define VIDEOPLAYER_ARTIST            278
#define VIDEOPLAYER_ALBUM             279
#define VIDEOPLAYER_WRITER            280
#define VIDEOPLAYER_TAGLINE           281
#define VIDEOPLAYER_HAS_INFO          282
#define VIDEOPLAYER_TOP250            283
#define VIDEOPLAYER_RATING_AND_VOTES  284
#define VIDEOPLAYER_TRAILER           285
#define VIDEOPLAYER_VIDEO_CODEC       286
#define VIDEOPLAYER_VIDEO_RESOLUTION  287
#define VIDEOPLAYER_AUDIO_CODEC       288
#define VIDEOPLAYER_AUDIO_CHANNELS    289
#define VIDEOPLAYER_VIDEO_ASPECT      290
#define VIDEOPLAYER_COUNTRY           292
#define VIDEOPLAYER_PLAYCOUNT         293
#define VIDEOPLAYER_LASTPLAYED        294

#define CONTAINER_SCROLL_PREVIOUS   345
#define CONTAINER_MOVE_PREVIOUS     346
#define CONTAINER_STATIC            347
#define CONTAINER_MOVE_NEXT         348
#define CONTAINER_SCROLL_NEXT       349

#define CONTAINER_CONTENT           362
#define CONTAINER_HAS_FOCUS         367
#define CONTAINER_ROW               368
#define CONTAINER_COLUMN            369
#define CONTAINER_POSITION          370
#define CONTAINER_VIEWMODE          371
#define CONTAINER_HAS_NEXT          372
#define CONTAINER_HAS_PREVIOUS      373
#define CONTAINER_SCROLLING         355
#define CONTAINER_NUM_ITEMS         359
#define CONTAINER_SUBITEM           374
#define CONTAINER_TVSHOWTHUMB       375
#define CONTAINER_NUM_PAGES         376
#define CONTAINER_CURRENT_PAGE      377
#define CONTAINER_SEASONTHUMB       378
#define CONTAINER_SHOWPLOT          379
#define CONTAINER_TOTALTIME         380

#define VISUALISATION_LOCKED         400
#define VISUALISATION_PRESET         401
#define VISUALISATION_NAME           402
#define VISUALISATION_ENABLED        403

#define SKIN_HAS_THEME_START        500
#define SKIN_HAS_THEME_END          599 // Allow for max 100 themes

#define SKIN_BOOL                   600
#define SKIN_STRING                 601
#define SKIN_HAS_MUSIC_OVERLAY      602
#define SKIN_HAS_VIDEO_OVERLAY      603

#define SYSTEM_FREE_MEMORY          648

#define SKIN_THEME                  800
#define SKIN_COLOUR_THEME           801

// Picture slideshow information
#define SLIDE_INFO_START            900
#define SLIDE_INFO_END              980

#define WINDOW_PROPERTY             9993
#define WINDOW_IS_TOPMOST           9994
#define WINDOW_IS_VISIBLE           9995
#define WINDOW_NEXT                 9996
#define WINDOW_PREVIOUS             9997
#define WINDOW_IS_MEDIA             9998
#define WINDOW_IS_ACTIVE            9999

#define CONTROL_IS_VISIBLE          29998
#define CONTROL_GROUP_HAS_FOCUS     29999

// The multiple information vector
#define MULTI_INFO_START            40000
#define MULTI_INFO_END              99999
#define COMBINED_VALUES_START       100000

#define CONTROL_HAS_FOCUS           30000

#define LISTITEM_START              35000
#define LISTITEM_THUMB              (LISTITEM_START)
#define LISTITEM_LABEL              (LISTITEM_START + 1)
#define LISTITEM_TITLE              (LISTITEM_START + 2)
#define LISTITEM_TRACKNUMBER        (LISTITEM_START + 3)
#define LISTITEM_ARTIST             (LISTITEM_START + 4)
#define LISTITEM_ALBUM              (LISTITEM_START + 5)
#define LISTITEM_YEAR               (LISTITEM_START + 6)
#define LISTITEM_GENRE              (LISTITEM_START + 7)
#define LISTITEM_ICON               (LISTITEM_START + 8)
#define LISTITEM_DIRECTOR           (LISTITEM_START + 9)
#define LISTITEM_OVERLAY            (LISTITEM_START + 10)
#define LISTITEM_LABEL2             (LISTITEM_START + 11)
#define LISTITEM_FILENAME           (LISTITEM_START + 12)
#define LISTITEM_DATE               (LISTITEM_START + 13)
#define LISTITEM_SIZE               (LISTITEM_START + 14)
#define LISTITEM_RATING             (LISTITEM_START + 15)
#define LISTITEM_PROGRAM_COUNT      (LISTITEM_START + 16)
#define LISTITEM_DURATION           (LISTITEM_START + 17)
#define LISTITEM_ISPLAYING          (LISTITEM_START + 18)
#define LISTITEM_ISSELECTED         (LISTITEM_START + 19)
#define LISTITEM_PLOT               (LISTITEM_START + 20)
#define LISTITEM_PLOT_OUTLINE       (LISTITEM_START + 21)
#define LISTITEM_EPISODE            (LISTITEM_START + 22)
#define LISTITEM_SEASON             (LISTITEM_START + 23)
#define LISTITEM_TVSHOW             (LISTITEM_START + 24)
#define LISTITEM_PREMIERED          (LISTITEM_START + 25)
#define LISTITEM_COMMENT            (LISTITEM_START + 26)
#define LISTITEM_ACTUAL_ICON        (LISTITEM_START + 27)
#define LISTITEM_PATH               (LISTITEM_START + 28)
#define LISTITEM_PICTURE_PATH       (LISTITEM_START + 29)
#define LISTITEM_PICTURE_DATETIME   (LISTITEM_START + 30)
#define LISTITEM_PICTURE_RESOLUTION (LISTITEM_START + 31)
#define LISTITEM_STUDIO             (LISTITEM_START + 32)
#define LISTITEM_MPAA               (LISTITEM_START + 33)
#define LISTITEM_CAST               (LISTITEM_START + 34)
#define LISTITEM_CAST_AND_ROLE      (LISTITEM_START + 35)
#define LISTITEM_WRITER             (LISTITEM_START + 36)
#define LISTITEM_TAGLINE            (LISTITEM_START + 37)
#define LISTITEM_TOP250             (LISTITEM_START + 38)
#define LISTITEM_RATING_AND_VOTES   (LISTITEM_START + 39)
#define LISTITEM_TRAILER            (LISTITEM_START + 40)
#define LISTITEM_STAR_RATING        (LISTITEM_START + 41)
#define LISTITEM_FILENAME_AND_PATH  (LISTITEM_START + 42)
#define LISTITEM_SORT_LETTER        (LISTITEM_START + 43)
#define LISTITEM_ALBUM_ARTIST       (LISTITEM_START + 44)
#define LISTITEM_FOLDERNAME         (LISTITEM_START + 45)
#define LISTITEM_VIDEO_CODEC        (LISTITEM_START + 46)
#define LISTITEM_VIDEO_RESOLUTION   (LISTITEM_START + 47)
#define LISTITEM_VIDEO_ASPECT       (LISTITEM_START + 48)
#define LISTITEM_AUDIO_CODEC        (LISTITEM_START + 49)
#define LISTITEM_AUDIO_CHANNELS     (LISTITEM_START + 50)
#define LISTITEM_AUDIO_LANGUAGE     (LISTITEM_START + 51)
#define LISTITEM_SUBTITLE_LANGUAGE  (LISTITEM_START + 52)
#define LISTITEM_IS_FOLDER          (LISTITEM_START + 53)
#define LISTITEM_ORIGINALTITLE      (LISTITEM_START + 54)
#define LISTITEM_COUNTRY            (LISTITEM_START + 55)
#define LISTITEM_PLAYCOUNT          (LISTITEM_START + 56)
#define LISTITEM_LASTPLAYED         (LISTITEM_START + 57)
#define LISTITEM_FOLDERPATH         (LISTITEM_START + 58)
#define LISTITEM_DISC_NUMBER        (LISTITEM_START + 59)
#define LISTITEM_FILE_EXTENSION     (LISTITEM_START + 60)
#define LISTITEM_IS_RESUMABLE       (LISTITEM_START + 61)

#define LISTITEM_PROPERTY_START     (LISTITEM_START + 200)
#define LISTITEM_PROPERTY_END       (LISTITEM_PROPERTY_START + 1000)
#define LISTITEM_END                (LISTITEM_PROPERTY_END)

#define MUSICPLAYER_PROPERTY_OFFSET 900 // last 100 id's reserved for musicplayer props.

#define CONDITIONAL_LABEL_START       LISTITEM_END + 1 // 36001
#define CONDITIONAL_LABEL_END         37000

// Info Flags
// Stored in the top 8 bits of GUIInfo::m_data1
// therefore we only have room for 8 flags
#define INFOFLAG_LISTITEM_WRAP        ((uint32_t) (1 << 25))  // Wrap ListItem lookups
#define INFOFLAG_LISTITEM_POSITION    ((uint32_t) (1 << 26))  // Absolute ListItem lookups

// Structure to hold multiple integer data
// for storage referenced from a single integer
class GUIInfo
{
public:
	GUIInfo(int info, uint32_t data1 = 0, int data2 = 0, uint32_t flag = 0)
	{
		m_info = info;
		m_data1 = data1;
		m_data2 = data2;
		if (flag)
			SetInfoFlag(flag);
	}
	bool operator ==(const GUIInfo &right) const
	{
		return (m_info == right.m_info && m_data1 == right.m_data1 && m_data2 == right.m_data2);
	};

	uint32_t GetInfoFlag() const;
	uint32_t GetData1() const;
	int GetData2() const;
	int m_info;

private:
	void SetInfoFlag(uint32_t flag);
	uint32_t m_data1;
	int m_data2;
};

class CGUIInfoManager
{
public:
	CGUIInfoManager(void);
	virtual ~CGUIInfoManager(void);
	
	void Clear();

	// Register a boolean condition/expression
	// This routine allows controls or other clients of the info manager to register
	// to receive updates of particular expressions, in a particular context (currently windows).
	// In the future, it will allow clients to receive pushed callbacks when the expression changes.
	// param expression the boolean condition or expression
	// param context the context window
	// return an identifier used to reference this expression
	// GetBoolValue
	unsigned int Register(const CStdString &expression, int context = 0);

	// Get a previously registered boolean expression's value
	// Checks the cache and evaluates the boolean expression if required.
	// Register
	bool GetBoolValue(unsigned int expression, const CGUIListItem *item = NULL);


	int TranslateString(const CStdString &strCondition);
	int TranslateSingleString(const CStdString &strCondition);

	int RegisterSkinVariableString(const INFO::CSkinVariableString* info);
	int TranslateSkinVariableString(const CStdString& name, int context);
	CStdString GetSkinVariableString(int info, bool preferImage = false, const CGUIListItem *item=NULL);

	// Evaluate a boolean expression
	// param expression the expression to evaluate
	// param context the context in which to evaluate the expression (currently windows)
	// return the value of the evaluated expression.
	// Register, GetBoolValue
	bool EvaluateBool(const CStdString &expression, int context = 0);

	CStdString GetItemLabel(const CFileItem *item, int info);
	CStdString GetItemImage(const CFileItem *item, int info);

	CStdString GetLabel(int info, int contextWindow = 0);
	CStdString GetImage(int info, int contextWindow);

	bool GetBool(int condition, int contextWindow = 0, const CGUIListItem *item = NULL);
	int GetInt(int info, int contextWindow = 0) const;
	void UpdateFPS();

	bool GetDisplayAfterSeek();
	void SetDisplayAfterSeek(unsigned int timeOut = 2500, int seekOffset = 0);
	void SetSeeking(bool seeking) { m_playerSeeking = seeking; };
	void SetShowCodec(bool showcodec) { m_playerShowCodec = showcodec; };
	void ToggleShowCodec() { m_playerShowCodec = !m_playerShowCodec; };
	void SetShowInfo(bool showinfo) { m_playerShowInfo = showinfo; };
	void ToggleShowInfo() { m_playerShowInfo = !m_playerShowInfo; };
	bool m_performingSeek;

	inline float GetFPS() const { return m_fps; };

	void SetNextWindow(int windowID) { m_nextWindowID = windowID; };
	void SetPreviousWindow(int windowID) { m_prevWindowID = windowID; };

	void ResetCache();
	
	__int64 GetPlayTime() const; // In ms
	CStdString GetCurrentPlayTime(TIME_FORMAT format = TIME_FORMAT_GUESS) const;
	int GetPlayTimeRemaining() const;
	CStdString GetCurrentPlayTimeRemaining(TIME_FORMAT format = TIME_FORMAT_GUESS) const;
	int GetTotalPlayTime() const;
	CStdString GetDuration(TIME_FORMAT format = TIME_FORMAT_GUESS) const;

	void ResetCurrentItem();
	void SetCurrentItem(CFileItem &item);
	void SetCurrentSong(CFileItem &item);
	void SetCurrentMovie(CFileItem &item);
	CStdString GetMusicLabel(int item);
	CStdString GetVideoLabel(int item);
	CStdString GetPictureLabel(int info);
	CStdString GetMusicTagLabel(int info, const CFileItem *item);

	void SetCurrentSlide(CFileItem &item);
	const CFileItem &GetCurrentSlide() const;
	void ResetCurrentSlide();

	void SetCurrentSongTag(const MUSIC_INFO::CMusicInfoTag &tag);
	void SetCurrentVideoTag(const CVideoInfoTag &tag);
	void SetCurrentAlbumThumb(const CStdString thumbFileName);
	const MUSIC_INFO::CMusicInfoTag *GetCurrentSongTag() const;
	const CVideoInfoTag* GetCurrentMovieTag() const;

	void SetContainerMoving(int id, bool next, bool scrolling)
	{
		m_containerMoves[id] = (next ? 1 : -1) * (scrolling ? 2 : 1);
	}

protected:
	// Routines for window retrieval
	bool CheckWindowCondition(CGUIWindow *window, int condition) const;
	CGUIWindow *GetWindowWithCondition(int contextWindow, int condition) const;

	bool GetMultiInfoBool(const GUIInfo &info, int contextWindow = 0);
	CStdString GetMultiInfoLabel(const GUIInfo &info, int contextWindow = 0);
	int TranslateListItem(const CStdString &info);
	TIME_FORMAT TranslateTimeFormat(const CStdString &format);
	bool GetItemBool(const CGUIListItem *item, int condition) const;

	CStdString GetTime(bool bSeconds = false);
	CStdString GetDate(bool bNumbersOnly = false);
	CStdString GetSystemHeatInfo(int info);

protected:
	// Conditional string parameters for testing are stored in a vector for later retrieval.
	// The offset into the string parameters array is returned.
	int ConditionalStringParameter(const CStdString &strParameter, bool caseSensitive = false);
	int AddMultiInfo(const GUIInfo &info);
	int AddListItemProp(const CStdString &str, int offset=0);

	// Conditional string parameters are stored here
	CStdStringArray m_stringParameters;

	// Fullscreen OSD Stuff
	DWORD m_AfterSeekTimeout;
	int m_seekOffset;
	bool m_playerSeeking;
	bool m_playerShowCodec;
	bool m_playerShowInfo;

	// FPS counters
	float m_fps;
	unsigned int m_frameCounter;
	unsigned int m_lastFPSTime;

	int m_nextWindowID;
	int m_prevWindowID;

	class CCombinedValue
	{
	public:
		CStdString m_info; // The text expression
		int m_id; // The id used to identify this expression
		std::list<int> m_postfix; // The postfix binary expression
		CCombinedValue& operator=(const CCombinedValue& mSrc);
	};
	
	std::vector<CCombinedValue> m_CombinedValues;

	int GetOperator(const char ch);
	int TranslateBooleanExpression(const CStdString &expression);
	bool EvaluateBooleanExpression(const CCombinedValue &expression, bool &result, int contextWindow);

	// Array of multiple information mapped to a single integer lookup
	std::vector<GUIInfo> m_multiInfo;
	std::vector<std::string> m_listitemProperties;

	CStdString m_currentMovieThumb;

	// Current playing stuff
	CFileItem* m_currentFile;
	CFileItem* m_currentSlide;
	unsigned int m_lastMusicBitrateTime;
	int m_MusicBitrate;

	std::vector<INFO::InfoBool*> m_bools;
	std::vector<INFO::CSkinVariableString> m_skinVariableStrings;
	unsigned int m_updateTime;
	std::map<int, int> m_containerMoves;

	CCriticalSection m_critInfo;
};

extern CGUIInfoManager g_infoManager;

#endif //CGUIINFOMANAGER_H

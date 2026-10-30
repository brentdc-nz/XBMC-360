#ifndef H_CVIDEOSETTINGS
#define H_CVIDEOSETTINGS

enum EINTERLACEMETHOD
{
	VS_INTERLACEMETHOD_NONE = 0,
	VS_INTERLACEMETHOD_AUTO = 1,
	VS_INTERLACEMETHOD_RENDER_BLEND = 2,

	VS_INTERLACEMETHOD_RENDER_WEAVE_INVERTED = 3,
	VS_INTERLACEMETHOD_RENDER_WEAVE = 4,
  
	VS_INTERLACEMETHOD_RENDER_BOB_INVERTED = 5,
	VS_INTERLACEMETHOD_RENDER_BOB = 6,

	VS_INTERLACEMETHOD_DEINTERLACE = 7
};

class CVideoSettings
{
public:
	CVideoSettings();
	~CVideoSettings() {};

	bool operator!=(const CVideoSettings &right) const;

	bool m_NoCache;
	bool m_NonInterleaved;
	bool m_bForceIndex;
	EINTERLACEMETHOD m_InterlaceMethod;
	float m_FilmGrain;
	int m_ViewMode;   // Current view mode
	float m_CustomZoomAmount; // Custom setting zoom amount
	float m_CustomPixelRatio; // Custom setting pixel ratio
	int m_AudioStream;
	float m_VolumeAmplification;
	bool m_OutputToAllSpeakers;
	int m_SubtitleStream;
	float m_SubtitleDelay;
	bool m_SubtitleOn;
	bool m_SubtitleCached;
	float m_Brightness;
	float m_Contrast;
	float m_Gamma;
	float m_NoiseReduction;
	bool m_PostProcess;
	float m_Sharpness;
	float m_AudioDelay;
	int m_ResumeTime;
	bool m_Crop;
	int m_CropTop;
	int m_CropBottom;
	int m_CropLeft;
	int m_CropRight;
};

#endif //H_CVIDEOSETTINGS
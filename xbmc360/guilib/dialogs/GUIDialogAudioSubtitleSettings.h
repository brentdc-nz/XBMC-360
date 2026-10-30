#ifndef GUILIB_GUIDIALOGAUDIOSUBTITLESETTINGS_H
#define GUILIB_GUIDIALOGAUDIOSUBTITLESETTINGS_H

#include "guilib\dialogs\GUIDialogSettings.h"

class CGUIDialogAudioSubtitleSettings :
			public CGUIDialogSettings
{
public:
	CGUIDialogAudioSubtitleSettings(void);
	virtual ~CGUIDialogAudioSubtitleSettings(void);
	virtual void FrameMove();

	static CStdString FormatDelay(float value, float minimum);
	static CStdString FormatDecibel(float value, float minimum);

protected:
	virtual void CreateSettings();
	virtual void OnSettingChanged(SettingInfo &setting);

	void AddAudioStreams(unsigned int id);
	void AddSubtitleStreams(unsigned int id);

	float m_volume;
	int m_audioStream;
	int m_subtitleStream;
	int m_outputmode;
	bool m_subtitleVisible;
};

#endif //GUILIB_GUIDIALOGAUDIOSUBTITLESETTINGS_H

#ifndef GUILIB_GUIDIALOGVIDEOSETTINGS_H
#define GUILIB_GUIDIALOGVIDEOSETTINGS_H

#include "guilib\dialogs\GUIDialogSettings.h"

class CGUIDialogVideoSettings :
			public CGUIDialogSettings
{
public:
	CGUIDialogVideoSettings(void);
	virtual ~CGUIDialogVideoSettings(void);

	static CStdString FormatInteger(float value, float minimum);
	static CStdString FormatFloat(float value, float minimum);

protected:
	virtual void CreateSettings();
	virtual void OnSettingChanged(SettingInfo &setting);

	int m_flickerFilter;
	bool m_soften;
};

#endif //GUILIB_GUIDIALOGVIDEOSETTINGS_H

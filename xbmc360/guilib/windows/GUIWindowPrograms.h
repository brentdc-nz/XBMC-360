#ifndef GUILIB_GUIWINDOWPROGRAMS_H
#define GUILIB_GUIWINDOWPROGRAMS_H

#include "..\GUIWindow.h"
#include "..\GUIMediaWindow.h"
#include "..\..\ThumbLoader.h"

class CGUIWindowPrograms : public CGUIMediaWindow
{
public:
	CGUIWindowPrograms(void);
	virtual ~CGUIWindowPrograms(void);

	virtual bool OnMessage(CGUIMessage& message);
	virtual bool GetDirectory(const CStdString &strDirectory, CFileItemList &items);
	bool Update(const CStdString &strDirectory);
	bool OnPlayMedia(int iItem);

private:
	CProgramThumbLoader m_thumbLoader;
};

#endif //GUILIB_GUIWINDOWPROGRAMS_H
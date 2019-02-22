#ifndef GUILIB_GUIMEDIAWINDOW_H
#define GUILIB_GUIMEDIAWINDOW_H

#include "GUIWindow.h"
#include "GUIViewControl.h"
#include "..\FileItem.h"

// Base class for all media windows
class CGUIMediaWindow : public CGUIWindow
{
public:
	CGUIMediaWindow(int id, const CStdString &xmlFile);
	virtual ~CGUIMediaWindow(void);

	virtual void OnWindowLoaded();
	virtual void OnWindowUnload();
	virtual void OnInitWindow();

	virtual bool OnMessage(CGUIMessage& message);

	bool OnClick(int iItem);
	virtual bool OnPlayMedia(int iItem);

protected:
	CGUIViewControl m_viewControl;
	virtual void OnSort();
	virtual bool GetDirectory(const CStdString &strDirectory, CFileItemList &items);
	virtual bool Update(const CStdString &strDirectory);

	virtual void ClearFileItems(); // Cleanup of items

	// Current path and history
	CFileItemList m_vecItems;
};

#endif //GUILIB_GUIMEDIAWINDOW
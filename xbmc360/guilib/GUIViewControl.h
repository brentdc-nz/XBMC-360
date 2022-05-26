#ifndef GUILIB_GUIVIEWCONTROL_H
#define GUILIB_GUIVIEWCONTROL_H

#include "GUIControl.h"
#include "..\FileItem.h"
#include "viewstates\GUIViewState.h"
#include <map>

typedef std::map<VIEW_METHOD, CGUIControl *>::const_iterator map_iter;

class CGUIViewControl
{
public:
	CGUIViewControl(void);
	virtual ~CGUIViewControl(void);

	void Reset();
	void SetParentWindow(int window);
	void SetCurrentView(VIEW_METHOD viewMode);
	void SetItems(CFileItemList &items);
	void SetSelectedItem(const CStdString &itemPath);
	void SetSelectedItem(int item);
	void AddView(VIEW_METHOD type, const CGUIControl *control);
	void SetViewControlID(int control);
	void SetFocused();
	bool HasControl(int viewControlID);
	int GetSelectedItem() const;
	int GetSelectedItem(const CGUIControl *control) const;
	bool HasViewMode(VIEW_METHOD viewMode);
	int GetCurrentControl();
	void Clear();

protected:
	void UpdateContents(const CGUIControl *control);
	void UpdateView();
	void UpdateViewAsControl();

	VIEW_METHOD m_currentView;
	map<VIEW_METHOD, CGUIControl*> m_vecViews;
	CFileItemList* m_fileItems;
	int m_viewAsControl;
	int m_parentWindow;
};

#endif //GUILIB_GUIVIEWCONTROL_H
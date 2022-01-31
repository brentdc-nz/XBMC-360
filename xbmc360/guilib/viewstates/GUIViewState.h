#ifndef H_CGUIVIEWSTATE
#define H_CGUIVIEWSTATE

#include "..\..\FileItem.h"

typedef enum {
	VIEW_METHOD_NONE = -1,
	VIEW_METHOD_LIST,
	VIEW_METHOD_THUMBS
} VIEW_METHOD;

class CGUIViewState
{
public:
	static CGUIViewState* GetViewState(int windowId, const CFileItemList& items);
	virtual ~CGUIViewState();

	VIEW_METHOD SetNextViewAsControl();
	VIEW_METHOD GetViewAsControl() const;
	virtual VECSOURCES& GetShares();
	virtual CStdString GetExtensions();

protected:
	CGUIViewState(const CFileItemList& items); // No direct object creation, use GetViewState()
	void AddViewAsControl(VIEW_METHOD viewAsControl, int buttonLabel);
	void SetViewAsControl(VIEW_METHOD viewAsControl);

	const CFileItemList& m_items;
	static VECSOURCES m_shares;

private:
	typedef struct _VIEW
	{
		VIEW_METHOD m_viewAsControl;
		int m_buttonLabel;
	} VIEW;

	vector<VIEW> m_viewAsControls;
	int m_currentViewAsControl;
};

class CGUIViewStateGeneral : public CGUIViewState
{
public:
	CGUIViewStateGeneral(const CFileItemList& items);

protected:
	virtual void SaveViewState() {};
};

#endif //H_CGUIVIEWSTATE
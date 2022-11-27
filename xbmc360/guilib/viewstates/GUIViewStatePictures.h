#ifndef H_CGUIVIEWSTATEPICTURES
#define H_CGUIVIEWSTATEPICTURES

#include "GUIViewState.h"

class CGUIViewStateWindowPictures : public CGUIViewState
{
public:
	CGUIViewStateWindowPictures(const CFileItemList& items);

protected:
	virtual void SaveViewState();
	virtual CStdString GetLockType();
	virtual bool UnrollArchives();
	virtual CStdString GetExtensions();
	virtual VECSOURCES& GetSources();
};

#endif //H_CGUIVIEWSTATEPICTURES
#ifndef H_CGUIVIEWSTATEPICTURES
#define H_CGUIVIEWSTATEPICTURES

#include "GUIViewState.h"

class CGUIViewStateWindowPictures : public CGUIViewState
{
public:
	CGUIViewStateWindowPictures(const CFileItemList& items);

protected:
	virtual VECSOURCES& GetShares();
	virtual CStdString GetExtensions();
};

#endif //H_CGUIVIEWSTATEPICTURES
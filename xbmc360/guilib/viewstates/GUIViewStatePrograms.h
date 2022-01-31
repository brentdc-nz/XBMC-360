#ifndef H_CGUIVIEWSTATEPROGRAMS
#define H_CGUIVIEWSTATEPROGRAMS

#include "GUIViewState.h"

class CGUIViewStateWindowPrograms : public CGUIViewState
{
public:
	CGUIViewStateWindowPrograms(const CFileItemList& items);

protected:
	virtual VECSOURCES& GetShares();
	virtual CStdString GetExtensions();
};

#endif //H_CGUIVIEWSTATEPROGRAMS
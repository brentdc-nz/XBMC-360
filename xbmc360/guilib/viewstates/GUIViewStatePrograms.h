#ifndef H_CGUIVIEWSTATEPROGRAMS
#define H_CGUIVIEWSTATEPROGRAMS

#include "GUIViewState.h"

class CGUIViewStateWindowPrograms : public CGUIViewState
{
public:
	CGUIViewStateWindowPrograms(const CFileItemList& items);

protected:
	virtual void SaveViewState();
	virtual CStdString GetLockType();
	virtual CStdString GetExtensions();
	virtual VECSOURCES& GetSources();
};

#endif //H_CGUIVIEWSTATEPROGRAMS
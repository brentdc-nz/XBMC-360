#ifndef H_CGUIVIEWSTATESCRIPTS
#define H_CGUIVIEWSTATESCRIPTS

#include "GUIViewState.h"

class CGUIViewStateWindowScripts : public CGUIViewState
{
public:
	CGUIViewStateWindowScripts(const CFileItemList& items);

protected:
	virtual void SaveViewState();
	virtual CStdString GetExtensions();
	virtual VECSOURCES& GetSources();
};

#endif //H_CGUIVIEWSTATESCRIPTS

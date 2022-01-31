#ifndef H_CGUIVIEWSTATEVIDEOS
#define H_CGUIVIEWSTATEVIDEOS

#include "GUIViewState.h"

class CGUIViewStateWindowVideos : public CGUIViewState
{
public:
	CGUIViewStateWindowVideos(const CFileItemList& items);

protected:
	virtual VECSOURCES& GetShares();
	virtual CStdString GetExtensions();
};

#endif //H_CGUIVIEWSTATEVIDEOS
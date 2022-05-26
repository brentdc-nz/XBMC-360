#ifndef H_CGUIVIEWSTATEMUSIC
#define H_CGUIVIEWSTATEMUSIC

#include "GUIViewState.h"

class CGUIViewStateWindowMusic : public CGUIViewState
{
public:
	CGUIViewStateWindowMusic(const CFileItemList& items);

protected:
	virtual VECSOURCES& GetShares();
	virtual CStdString GetExtensions();
};

#endif //H_CGUIVIEWSTATEMUSIC
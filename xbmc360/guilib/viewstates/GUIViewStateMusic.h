#ifndef H_CGUIVIEWSTATEMUSIC
#define H_CGUIVIEWSTATEMUSIC

#include "GUIViewState.h"
#include "FileItem.h"

class CGUIViewStateWindowMusicFiles : public CGUIViewState
{
public:
	CGUIViewStateWindowMusicFiles(const CFileItemList& items);
	virtual CStdString GetExtensions();

protected:
	virtual CStdString GetLockType();
	virtual void SaveViewState();
	virtual VECSOURCES& GetSources();
};

#endif //H_CGUIVIEWSTATEMUSIC
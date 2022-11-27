#ifndef H_CGUIVIEWSTATEVIDEOS
#define H_CGUIVIEWSTATEVIDEOS

#include "GUIViewState.h"
#include "FileItem.h"

class CGUIViewStateWindowVideo : public CGUIViewState
{
public:
	CGUIViewStateWindowVideo(const CFileItemList& items) : CGUIViewState(items) {}

protected:
	virtual CStdString GetLockType();
	virtual bool UnrollArchives();
	virtual int GetPlaylist();
	virtual CStdString GetExtensions();
};

class CGUIViewStateWindowVideoFiles : public CGUIViewStateWindowVideo
{
public:
	CGUIViewStateWindowVideoFiles(const CFileItemList& items);

protected:
	virtual void SaveViewState();
	virtual VECSOURCES& GetSources();
};

#endif //H_CGUIVIEWSTATEVIDEOS
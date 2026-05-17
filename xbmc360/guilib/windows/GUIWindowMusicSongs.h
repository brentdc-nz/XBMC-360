#ifndef GUILIB_GUIWINDOWMUSICSONGS_H
#define GUILIB_GUIWINDOWMUSICSONGS_H

#include "guilib\GUIMediaWindow.h"
#include "ThumbLoader.h"
#include "music\MusicInfoLoader.h"

class CGUIDialogProgress;

class CGUIWindowMusicSongs : public CGUIMediaWindow
{
public:
	CGUIWindowMusicSongs(void);
	virtual ~CGUIWindowMusicSongs(void);
	
	virtual bool OnMessage(CGUIMessage& message);

protected:
	virtual void GetContextButtons(int itemNumber, CContextButtons &buttons);
	virtual bool OnContextButton(int itemNumber, CONTEXT_BUTTON button);
	virtual bool Update(const CStdString &strDirectory);
	virtual void OnPrepareFileItems(CFileItemList &items);

	void RetrieveMusicInfo();
	void OnRetrieveMusicInfo(CFileItemList& items);

	CGUIDialogProgress* m_dlgProgress;
	CMusicThumbLoader m_thumbLoader;
	MUSIC_INFO::CMusicInfoLoader m_musicInfoLoader;
};

#endif //GUILIB_GUIWINDOWMUSICSONGS_H

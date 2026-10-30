#include "utils\Log.h"
#include "guilib\dialogs\GUIDialogVideoBookmarks.h"
#include "video\VideoDatabase.h"
#include "Application.h"
#include "utils\Util.h"
#include "utils\URIUtils.h"
#include "cores\VideoRenderers\RenderManager.h"
#include "pictures\Picture.h"
#include "guilib\dialogs\GUIDialogContextMenu.h"
#include "guilib\GUIWindowManager.h"
#include "ViewState.h"
#include "Settings.h"
#include "AdvancedSettings.h"
#include "FileItem.h"
#include "utils\Crc32.h"
#include "utils\SingleLock.h"
#include "utils\StringUtils.h"
#include "guilib\GraphicContext.h"
#include "cores\PlayerCoreFactory.h"
#include "guilib\LocalizeStrings.h"

using namespace std;

#define BOOKMARK_THUMB_WIDTH g_advancedSettings.m_thumbSize

#define CONTROL_ADD_BOOKMARK           2
#define CONTROL_CLEAR_BOOKMARKS        3
#define CONTROL_ADD_EPISODE_BOOKMARK   4

#define CONTROL_LIST                  10
#define CONTROL_THUMBS                11

CGUIDialogVideoBookmarks::CGUIDialogVideoBookmarks()
	: CGUIDialog(WINDOW_DIALOG_VIDEO_BOOKMARKS, "VideoOSDBookmarks.xml")
{
	m_vecItems = new CFileItemList;
}

CGUIDialogVideoBookmarks::~CGUIDialogVideoBookmarks()
{
	delete m_vecItems;
}

bool CGUIDialogVideoBookmarks::OnMessage(CGUIMessage& message)
{
	switch ( message.GetMessage() )
	{
	case GUI_MSG_WINDOW_DEINIT:
		{
			// TODO: Video database directory cache not ported - xbmc4xbox: CUtil::DeleteVideoDatabaseDirectoryCache();
			Clear();
		}
		break;

	case GUI_MSG_WINDOW_INIT:
		{
			CGUIWindow::OnMessage(message);
			Update();
			return true;
		}
		break;

	case GUI_MSG_CLICKED:
		{
			int iControl = message.GetSenderId();
			if (iControl == CONTROL_ADD_BOOKMARK)
			{
				AddBookmark();
			}
			else if (iControl == CONTROL_CLEAR_BOOKMARKS)
			{
				ClearBookmarks();
			}
			else if (iControl == CONTROL_ADD_EPISODE_BOOKMARK)
			{
				AddEpisodeBookmark();
			}
			else if (m_viewControl.HasControl(iControl))  // list/thumb control
			{
				int iItem = m_viewControl.GetSelectedItem();
				int iAction = message.GetParam1();
				if (iAction == ACTION_DELETE_ITEM)
				{
					if( (unsigned)iItem < m_bookmarks.size() )
					{
						CVideoDatabase videoDatabase;
						videoDatabase.Open();
						videoDatabase.ClearBookMarkOfFile(g_application.CurrentFile(),m_bookmarks[iItem],m_bookmarks[iItem].type);
						videoDatabase.Close();
						// TODO: Video database directory cache not ported - xbmc4xbox: CUtil::DeleteVideoDatabaseDirectoryCache();
					}
					Update();
				}
				else if (iAction == ACTION_SELECT_ITEM || iAction == ACTION_MOUSE_LEFT_CLICK)
				{
					GotoBookmark(iItem);
				}
			}
		}
		break;
	case GUI_MSG_SETFOCUS:
		{
			if (m_viewControl.HasControl(message.GetControlId()) && m_viewControl.GetCurrentControl() != message.GetControlId())
			{
				m_viewControl.SetFocused();
				return true;
			}
		}
		break;
	}

	return CGUIDialog::OnMessage(message);
}

void CGUIDialogVideoBookmarks::Update()
{
	m_bookmarks.clear();
	CBookmark resumemark;

	// open the d/b and retrieve the bookmarks for the current movie
	CVideoDatabase videoDatabase;
	videoDatabase.Open();
	videoDatabase.GetBookMarksForFile(g_application.CurrentFile(), m_bookmarks);
	videoDatabase.GetBookMarksForFile(g_application.CurrentFile(), m_bookmarks, CBookmark::EPISODE, true);
	/* push in the resume mark first */
	if( videoDatabase.GetResumeBookMark(g_application.CurrentFile(), resumemark) )
		m_bookmarks.insert(m_bookmarks.begin(), resumemark);

	// TODO: Episode bookmarks not ported (no GetEpisodesByFile in VideoDatabase) - xbmc4xbox:
	// if (g_application.CurrentFileItem().HasVideoInfoTag() && g_application.CurrentFileItem().GetVideoInfoTag()->m_iEpisode > -1)
	// {
	//   vector<CVideoInfoTag> episodes;
	//   videoDatabase.GetEpisodesByFile(g_application.CurrentFile(),episodes);
	//   if (episodes.size() > 1)
	//   {
	//     CONTROL_ENABLE(CONTROL_ADD_EPISODE_BOOKMARK);
	//   }
	//   else
	//   {
	//     CONTROL_DISABLE(CONTROL_ADD_EPISODE_BOOKMARK);
	//   }
	// }
	// else
	// {
	//   CONTROL_DISABLE(CONTROL_ADD_EPISODE_BOOKMARK);
	// }
	CONTROL_DISABLE(CONTROL_ADD_EPISODE_BOOKMARK);

	videoDatabase.Close();

	// lock our display, as this window is rendered from the player thread
	g_graphicsContext.Lock();
	m_viewControl.SetCurrentView(DEFAULT_VIEW_ICONS);

	// empty the list ready for population
	Clear();

	// cycle through each stored bookmark and add it to our list control
	for (unsigned int i = 0; i < m_bookmarks.size(); ++i)
	{
		if (m_bookmarks[i].type == CBookmark::RESUME)
			m_bookmarks[i].thumbNailImage = "bookmark-resume.png";

		CStdString bookmarkTime;
		if (m_bookmarks[i].type == CBookmark::EPISODE)
			bookmarkTime.Format("%s %i %s %i", g_localizeStrings.Get(20373), m_bookmarks[i].seasonNumber, g_localizeStrings.Get(20359).c_str(), m_bookmarks[i].episodeNumber);
		else
			bookmarkTime = CStringUtils::SecondsToTimeString((long)m_bookmarks[i].timeInSeconds, TIME_FORMAT_HH_MM_SS);

		CFileItemPtr item(new CFileItem(bookmarkTime));
		item->SetThumbnailImage(m_bookmarks[i].thumbNailImage);
		m_vecItems->Add(item);
	}
	m_viewControl.SetItems(*m_vecItems);
	g_graphicsContext.Unlock();
}

void CGUIDialogVideoBookmarks::Clear()
{
	m_viewControl.Clear();
	m_vecItems->Clear();
}

void CGUIDialogVideoBookmarks::GotoBookmark(int item)
{
	if (item < 0 || item >= (int)m_bookmarks.size()) return;
	if (g_application.m_pPlayer)
	{
		g_application.m_pPlayer->SetPlayerState(m_bookmarks[item].playerState);
		g_application.SeekTime((double)m_bookmarks[item].timeInSeconds);
	}
}

void CGUIDialogVideoBookmarks::ClearBookmarks()
{
	CVideoDatabase videoDatabase;
	videoDatabase.Open();
	videoDatabase.ClearBookMarksOfFile(g_application.CurrentFile(), CBookmark::STANDARD);
	videoDatabase.ClearBookMarksOfFile(g_application.CurrentFile(), CBookmark::RESUME);
	videoDatabase.ClearBookMarksOfFile(g_application.CurrentFile(), CBookmark::EPISODE);
	videoDatabase.Close();
	Update();
}

void CGUIDialogVideoBookmarks::AddBookmark(CVideoInfoTag* tag)
{
	CVideoDatabase videoDatabase;
	CBookmark bookmark;
	bookmark.timeInSeconds = (int)g_application.GetTime();
	bookmark.totalTimeInSeconds = (int)g_application.GetTotalTime();

	if( g_application.m_pPlayer )
		bookmark.playerState = g_application.m_pPlayer->GetPlayerState();
	else
		bookmark.playerState.Empty();

	bookmark.player = CPlayerCoreFactory::GetPlayerName(g_application.GetCurrentPlayer());

	// TODO: Bookmark thumbnails not ported (no g_renderManager.CreateThumbnail / GetBookmarksThumbFolder) - xbmc4xbox:
	// // create the thumbnail image
	// float aspectRatio = g_renderManager.GetAspectRatio();
	// int width = BOOKMARK_THUMB_WIDTH;
	// int height = (int)(BOOKMARK_THUMB_WIDTH / aspectRatio);
	// if (height > BOOKMARK_THUMB_WIDTH)
	// {
	//   height = BOOKMARK_THUMB_WIDTH;
	//   width = (int)(BOOKMARK_THUMB_WIDTH * aspectRatio);
	// }
	// CSingleLock lock(g_graphicsContext);
	// LPDIRECT3DTEXTURE8 texture = NULL;
	// if (D3D_OK == D3DXCreateTexture(g_graphicsContext.Get3DDevice(), width, height, 1, 0, D3DFMT_LIN_A8R8G8B8, D3DPOOL_MANAGED, &texture))
	// {
	//   LPDIRECT3DSURFACE8 surface = NULL;
	//   texture->GetSurfaceLevel(0, &surface);
	//   g_renderManager.CreateThumbnail(surface, width, height);
	//   D3DLOCKED_RECT lockedRect;
	//   surface->LockRect(&lockedRect, NULL, NULL);
	//   // compute the thumb name + create the thumb image
	//   Crc32 crc;
	//   crc.ComputeFromLowerCase(g_application.CurrentFile());
	//   bookmark.thumbNailImage.Format("%08x_%i.jpg", (unsigned __int32) crc, m_vecItems->Size() + 1);
	//   bookmark.thumbNailImage = URIUtils::AddFileToFolder(g_settings.GetBookmarksThumbFolder(), bookmark.thumbNailImage);
	//   CPicture pic;
	//   if (!pic.CreateThumbnailFromSurface((BYTE *)lockedRect.pBits, width, height, lockedRect.Pitch, bookmark.thumbNailImage))
	//     bookmark.thumbNailImage.Empty();
	//   surface->UnlockRect();
	//   surface->Release();
	//   texture->Release();
	// }
	// lock.Leave();
	bookmark.thumbNailImage.Empty();

	videoDatabase.Open();
	// TODO: Episode bookmarks not ported (no AddBookMarkForEpisode in VideoDatabase) - xbmc4xbox:
	// if (tag)
	//   videoDatabase.AddBookMarkForEpisode(*tag, bookmark);
	// else
	videoDatabase.AddBookMarkToFile(g_application.CurrentFile(), bookmark, CBookmark::STANDARD);
	videoDatabase.Close();
	Update();
}

void CGUIDialogVideoBookmarks::OnWindowLoaded()
{
	CGUIDialog::OnWindowLoaded();
	m_viewControl.Reset();
	m_viewControl.SetParentWindow(GetID());
	m_viewControl.AddView(GetControl(CONTROL_THUMBS));
}

void CGUIDialogVideoBookmarks::OnWindowUnload()
{
	CGUIDialog::OnWindowUnload();
	m_viewControl.Reset();
}

CGUIControl *CGUIDialogVideoBookmarks::GetFirstFocusableControl(int id)
{
	if (m_viewControl.HasControl(id))
		id = m_viewControl.GetCurrentControl();
	return CGUIWindow::GetFirstFocusableControl(id);
}

void CGUIDialogVideoBookmarks::AddEpisodeBookmark()
{
	// TODO: Episode bookmarks not ported (no GetEpisodesByFile in VideoDatabase) - xbmc4xbox:
	// vector<CVideoInfoTag> episodes;
	// CVideoDatabase videoDatabase;
	// videoDatabase.Open();
	// videoDatabase.GetEpisodesByFile(g_application.CurrentFile(), episodes);
	// videoDatabase.Close();
	// if(episodes.size() > 0)
	// {
	//   CContextButtons choices;
	//   for (unsigned int i=0; i < episodes.size(); ++i)
	//   {
	//     CStdString strButton;
	//     strButton.Format("%s %i, %s %i", g_localizeStrings.Get(20373), episodes[i].m_iSeason, g_localizeStrings.Get(20359).c_str(), episodes[i].m_iEpisode);
	//     choices.Add(i, strButton);
	//   }
	//
	//   int pressed = CGUIDialogContextMenu::ShowAndGetChoice(choices);
	//   if (pressed >= 0)
	//     AddBookmark(&episodes[pressed]);
	// }
}

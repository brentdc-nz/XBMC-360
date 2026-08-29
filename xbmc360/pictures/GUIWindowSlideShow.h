#pragma once
#include <set>
#include "guilib/GUIWindow.h"
#include "utils/Thread.h"
#include "utils/CriticalSection.h"
#include "pictures/SlideShowPicture.h"
#include "SortFileItem.h"

class CFileItemList;

class CGUIWindowSlideShow;

class CBackgroundPicLoader : public CThread
{
public:
	CBackgroundPicLoader();
	~CBackgroundPicLoader();

	void Create(CGUIWindowSlideShow *pCallback);
	void LoadPic(int iPic, int iSlideNumber, const CStdString &strFileName, const int maxWidth, const int maxHeight);
	bool IsLoading() { return m_isLoading;};

private:
	void Process();
	int m_iPic;
	int m_iSlideNumber;
	CStdString m_strFileName;
	int m_maxWidth;
	int m_maxHeight;

	HANDLE m_loadPic;
	bool m_isLoading;

	CGUIWindowSlideShow *m_pCallback;
};

class CGUIWindowSlideShow : public CGUIWindow
{
public:
	CGUIWindowSlideShow(void);
	virtual ~CGUIWindowSlideShow(void);

	void Reset();
	void Add(const CFileItem *picture);
	bool IsPlaying() const;
	void ShowNext();
	void ShowPrevious();
	void Select(const CStdString& strPicture);
	const CFileItemList &GetSlideShowContents();
	const CFileItemPtr GetCurrentSlide();
	void RunSlideShow(const CStdString& strPath, bool bRecursive = false,
										bool bRandom = false, bool bNotRandom = false,
										SORT_METHOD method = SORT_METHOD_LABEL,
										SORT_ORDER order = SORT_ORDER_ASC);
	void AddFromPath(const CStdString &strPath, bool bRecursive,
									 SORT_METHOD method=SORT_METHOD_LABEL,
									 SORT_ORDER order=SORT_ORDER_ASC);
	void StartSlideShow(bool screensaver=false);
	bool InSlideShow() const;
	virtual bool OnMessage(CGUIMessage& message);
	virtual bool OnAction(const CAction &action);
	virtual void Render();
	virtual void FreeResources();
	void OnLoadPic(int iPic, int iSlideNumber, LPDIRECT3DTEXTURE9 pTexture, int iWidth, int iHeight, int iOriginalWidth, int iOriginalHeight, int iRotate, bool bFullSize);
	int NumSlides() const;
	int CurrentSlide() const;
	void Shuffle();
private:
	typedef std::set<CStdString> path_set;  // set to track which paths we're adding
	void AddItems(const CStdString &strPath, path_set *recursivePaths,
								SORT_METHOD method=SORT_METHOD_LABEL,
								SORT_ORDER order=SORT_ORDER_ASC);
	void RenderPause();
	void RenderErrorMessage();
	void Rotate();
	void Zoom(int iZoom);
	void Move(float fX, float fY);
	void GetCheckedSize(float width, float height, int &maxWidth, int &maxHeight);

	int m_iCurrentSlide;
	int m_iNextSlide;
	int m_iRotate;
	int m_iZoomFactor;

	bool m_bSlideShow;
	bool m_bScreensaver;
	bool m_bPause;
	bool m_bErrorMessage;

	CFileItemList* m_slides;

	CSlideShowPic m_Image[2];

	int m_iCurrentPic;
	// Background loader
	CBackgroundPicLoader* m_pBackgroundLoader;
	bool m_bWaitForNextPic;
	bool m_bLoadNextPic;
	bool m_bReloadImage;
	RESOLUTION m_Resolution;
	CCriticalSection m_slideSection;
};

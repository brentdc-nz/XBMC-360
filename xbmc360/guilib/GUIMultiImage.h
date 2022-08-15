#ifndef GUILIB_GUIMULTIIMAGECONTROL_H
#define GUILIB_GUIMULTIIMAGECONTROL_H

#include "GUIImage.h"
#include "utils/Stopwatch.h"

class CGUIMultiImage : public CGUIControl
{
public:
	CGUIMultiImage(int parentID, int controlID, float posX, float posY, float width, float height, const CTextureInfo& texture, unsigned int timePerImage, unsigned int fadeTime, bool randomized, bool loop, unsigned int timeToPauseAtEnd);
	virtual ~CGUIMultiImage(void);
	
	virtual void AllocResources();
	virtual void FreeResources(bool immediately = false);
	virtual void Render();
	virtual bool OnAction(const CAction &action);
	void SetInfo(const CGUIInfoLabel &info);
	virtual bool CanFocus() const;

protected:
	void LoadDirectory();

	CGUIInfoLabel m_texturePath;
	CStdString m_currentPath;
	unsigned int m_currentImage;
	CStopWatch m_imageTimer;
	unsigned int m_timePerImage;
	unsigned int m_timeToPauseAtEnd;
	bool m_randomized;
	bool m_loop;

	bool m_directoryLoaded;
	std::vector<CStdString> m_files;

	CGUIImage m_image;
};

#endif //GUILIB_GUIMULTIIMAGECONTROL_H
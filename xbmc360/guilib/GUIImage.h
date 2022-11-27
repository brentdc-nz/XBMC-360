#ifndef GUILIB_GUIIMAGECONTROL_H
#define GUILIB_GUIIMAGECONTROL_H

#include "GUIControl.h"
#include "GUITextureD3D.h"

class CGUIImage : public CGUIControl
{
public:
	class CFadingTexture
	{
		public:
		CFadingTexture(const CGUITexture &texture, unsigned int fadeTime)
		{
			// Create a copy of our texture, and allocate resources
			m_texture = new CGUITexture(texture);
			m_texture->AllocResources();
			m_fadeTime = fadeTime;
			m_fading = false;
		};

		~CFadingTexture()
		{
			m_texture->FreeResources();
			delete m_texture;
		};

		CGUITexture *m_texture;  // Texture to fade out
		unsigned int m_fadeTime; // Time to fade out (ms)
		bool         m_fading;   // Whether we're fading out
	};

	CGUIImage(int parentID, int controlID, float posX, float posY, float width, float height, const CTextureInfo& texture);
	CGUIImage(const CGUIImage &left);
	virtual ~CGUIImage(void);
	virtual CGUIImage *Clone() const { return new CGUIImage(*this); };

	virtual void Render();
	virtual void UpdateVisibility(const CGUIListItem *item = NULL);
	virtual bool OnAction(const CAction &action) ;
	virtual bool OnMessage(CGUIMessage& message);
	virtual void PreAllocResources();
	virtual void AllocResources();
	virtual void FreeResources(bool immediately = false);
	virtual void DynamicResourceAlloc(bool bOnOff);
	virtual bool IsDynamicallyAllocated() { return m_bDynamicResourceAlloc; };
	virtual bool CanFocus() const;
	virtual void UpdateInfo(const CGUIListItem *item = NULL);

	virtual void SetInfo(const CGUIInfoLabel &info);
	virtual void SetFileName(const CStdString& strFileName, bool setConstant = false);
	virtual void SetAspectRatio(const CAspectRatio &aspect);
	virtual void SetWidth(float width);
	virtual void SetHeight(float height);
	virtual void SetPosition(float posX, float posY);
	void SetCrossFade(unsigned int time);

	const CStdString& GetFileName() const;
	float GetTextureWidth() const;
	float GetTextureHeight() const;

#ifdef _DEBUG
	virtual void DumpTextureUse();
#endif

protected:
	virtual void AllocateOnDemand();
	virtual void FreeTextures(bool immediately = false);
	void FreeResourcesButNotAnims();
	unsigned char GetFadeLevel(unsigned int time) const;
	bool RenderFading(CFadingTexture *texture, unsigned int frameTime);

	bool m_bDynamicResourceAlloc;

	// Border + conditional info
	CTextureInfo m_image;
	CGUIInfoLabel m_info;

	CGUITexture m_texture;
	std::vector<CFadingTexture *> m_fadingTextures;
	CStdString m_currentTexture;

	unsigned int m_crossFadeTime;
	unsigned int m_currentFadeTime;
	unsigned int m_lastRenderTime;
};

#endif //GUILIB_GUIIMAGECONTROL_H
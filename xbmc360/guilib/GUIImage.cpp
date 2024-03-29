#include "include.h"
#include "GUIImage.h"
#include "TextureManager.h"
#include "utils/TimeUtils.h"

using namespace std;

CGUIImage::CGUIImage(int parentID, int controlID, float posX, float posY, float width, float height, const CTextureInfo& texture)
	: CGUIControl(parentID, controlID, posX, posY, width, height)
	, m_texture(posX, posY, width, height, texture)
{
	m_crossFadeTime = 0;
	m_currentFadeTime = 0;
	m_lastRenderTime = 0;
	ControlType = GUICONTROL_IMAGE;
	m_bDynamicResourceAlloc=false;
}

CGUIImage::CGUIImage(const CGUIImage &left)
	: CGUIControl(left), m_texture(left.m_texture)
{
	m_info = left.m_info;
	m_crossFadeTime = left.m_crossFadeTime;

	// Defaults
	m_currentFadeTime = 0;
	m_lastRenderTime = 0;
	ControlType = GUICONTROL_IMAGE;
	m_bDynamicResourceAlloc=false;
}

CGUIImage::~CGUIImage(void)
{
}

void CGUIImage::UpdateVisibility(const CGUIListItem *item)
{
	CGUIControl::UpdateVisibility(item);

	// Now that we've checked for conditional info, we can
	// check for allocation
	AllocateOnDemand();
}

void CGUIImage::UpdateInfo(const CGUIListItem *item)
{
	if (m_info.IsConstant())
		return; // Nothing to do

	// Don't allow image to change while animating out
	if (HasRendered() && IsAnimating(ANIM_TYPE_HIDDEN) && !IsVisibleFromSkin())
		return;

	if (item)
		SetFileName(m_info.GetItemLabel(item, true));
	else
		SetFileName(m_info.GetLabel(m_parentID, true));
}

void CGUIImage::AllocateOnDemand()
{
  // if we're hidden, we can free our resources and return
  if (!IsVisible() && m_visible != DELAYED)
  {
    if (m_bDynamicResourceAlloc && m_texture.IsAllocated())
      FreeResourcesButNotAnims();
    return;
  }

  // either visible or delayed - we need the resources allocated in either case
  if (!m_texture.IsAllocated())
    AllocResources();
}

void CGUIImage::Render()
{
	if (!IsVisible()) return;

	// Check whether our image failed to allocate, and if so drop back to the fallback image
	if (m_texture.FailedToAlloc() && !m_texture.GetFileName().Equals(m_info.GetFallback()))
		m_texture.SetFileName(m_info.GetFallback());

	if (m_crossFadeTime)
	{
		// Make sure our texture has started allocating
		m_texture.AllocResources();

		// Compute the frame time
		unsigned int frameTime = 0;
		unsigned int currentTime = CTimeUtils::GetFrameTime();
		
		if (m_lastRenderTime)
			frameTime = currentTime - m_lastRenderTime;

		m_lastRenderTime = currentTime;

		if (m_fadingTextures.size()) // Have some fading images
		{
			// Anything other than the last old texture needs to be faded out as per usual
			for (vector<CFadingTexture *>::iterator i = m_fadingTextures.begin(); i != m_fadingTextures.end() - 1;)
			{
				if (!RenderFading(*i, frameTime))
					i = m_fadingTextures.erase(i);
				else
					i++;
			}

			if (m_texture.ReadyToRender() || m_texture.GetFileName().IsEmpty())
			{
				// Fade out the last one as well
				if (!RenderFading(m_fadingTextures[m_fadingTextures.size() - 1], frameTime))
					m_fadingTextures.erase(m_fadingTextures.end() - 1);
			}
			else
			{
				// Keep the last one fading in
				CFadingTexture *texture = m_fadingTextures[m_fadingTextures.size() - 1];
				texture->m_fadeTime += frameTime;

				if (texture->m_fadeTime > m_crossFadeTime)
					texture->m_fadeTime = m_crossFadeTime;

				texture->m_texture->SetAlpha(GetFadeLevel(texture->m_fadeTime));
				texture->m_texture->SetDiffuseColor(m_diffuseColor);
				texture->m_texture->Render();
			}
		}

		if (m_texture.ReadyToRender() || m_texture.GetFileName().IsEmpty())
		{
			// Fade the new one in
			m_currentFadeTime += frameTime;

			if (m_currentFadeTime > m_crossFadeTime || frameTime == 0) // For if we allocate straight away on creation
				m_currentFadeTime = m_crossFadeTime;
		}
		m_texture.SetAlpha(GetFadeLevel(m_currentFadeTime));
	}

	m_texture.SetDiffuseColor(m_diffuseColor);
	m_texture.Render();

	CGUIControl::Render();
}

bool CGUIImage::RenderFading(CGUIImage::CFadingTexture *texture, unsigned int frameTime)
{
	assert(texture);

	if (texture->m_fadeTime <= frameTime)
	{
		// Time to kill off the texture
		delete texture;
		return false;
	}

	// Render this texture
	texture->m_fadeTime -= frameTime;
	texture->m_texture->SetAlpha(GetFadeLevel(texture->m_fadeTime));
	texture->m_texture->SetDiffuseColor(m_diffuseColor);
	texture->m_texture->Render();

	return true;
}

bool CGUIImage::OnAction(const CAction &action)
{
	return false;
}

bool CGUIImage::OnMessage(CGUIMessage& message)
{
	if (message.GetMessage() == GUI_MSG_REFRESH_THUMBS)
	{
		if (!m_info.IsConstant())
			FreeTextures(true); // True as we want to free the texture immediately

		return true;
	}
	return CGUIControl::OnMessage(message);
}

void CGUIImage::PreAllocResources()
{
	FreeResources();
	m_texture.PreAllocResources();
}

void CGUIImage::AllocResources()
{
	if (m_texture.GetFileName().IsEmpty())
		return;

	CGUIControl::AllocResources();
	m_texture.AllocResources();
}

void CGUIImage::FreeTextures(bool immediately /* = false */)
{
	m_texture.FreeResources(immediately);

	for (unsigned int i = 0; i < m_fadingTextures.size(); i++)
		delete m_fadingTextures[i];

	m_fadingTextures.clear();
	m_currentTexture.Empty();
}

void CGUIImage::FreeResources(bool immediately)
{
	FreeTextures(immediately);
	CGUIControl::FreeResources(immediately);
}

// WORKAROUND - We are currently resetting all animations when this is called, which shouldn't be the case
//              see CGUIControl::FreeResources() - This needs remedying
void CGUIImage::FreeResourcesButNotAnims()
{
	FreeTextures();
	m_bAllocated = false;
	m_hasRendered = false;
}

void CGUIImage::DynamicResourceAlloc(bool bOnOff)
{
	m_bDynamicResourceAlloc = bOnOff;
	m_texture.DynamicResourceAlloc(bOnOff);

	CGUIControl::DynamicResourceAlloc(bOnOff);
}

bool CGUIImage::CanFocus() const
{
	return false;
}

float CGUIImage::GetTextureWidth() const
{
	return m_texture.GetTextureWidth();
}

float CGUIImage::GetTextureHeight() const
{
	return m_texture.GetTextureHeight();
}

const CStdString &CGUIImage::GetFileName() const
{
	return m_texture.GetFileName();
}

void CGUIImage::SetAspectRatio(const CAspectRatio &aspect)
{
	m_texture.SetAspectRatio(aspect);
}

void CGUIImage::SetCrossFade(unsigned int time)
{
	m_crossFadeTime = time;

	if (!m_crossFadeTime && m_texture.IsLazyLoaded() && !m_info.GetFallback().IsEmpty())
		m_crossFadeTime = 1;
}

void CGUIImage::SetFileName(const CStdString& strFileName, bool setConstant)
{
	if (setConstant)
		m_info.SetLabel(strFileName, "", GetParentID());

	if (m_crossFadeTime)
	{
		// Set filename on the next texture
		if (m_currentTexture.Equals(strFileName))
			return; // Nothing to do - We already have this image

		if (m_texture.ReadyToRender() || m_texture.GetFileName().IsEmpty())
		{
			// Save the current image
			m_fadingTextures.push_back(new CFadingTexture(m_texture, m_currentFadeTime));
		}
		m_currentFadeTime = 0;
	}

	if (!m_currentTexture.Equals(strFileName))
	{
		// Texture is changing - Attempt to load it, and save the name in m_currentTexture
		// we'll check whether it loaded or not in Render()
		m_currentTexture = strFileName;
		m_texture.SetFileName(m_currentTexture);
	}
}

#ifdef _DEBUG
void CGUIImage::DumpTextureUse()
{
	if (m_texture.IsAllocated())
	{
		if (GetID())
			CLog::Log(LOGDEBUG, "Image control %u using texture %s",
					GetID(), m_texture.GetFileName().c_str());
		else
			CLog::Log(LOGDEBUG, "Using texture %s", m_texture.GetFileName().c_str());
	}
}
#endif

void CGUIImage::SetWidth(float width)
{
	m_texture.SetWidth(width);
	CGUIControl::SetWidth(m_texture.GetWidth());
}

void CGUIImage::SetHeight(float height)
{
	m_texture.SetHeight(height);
	CGUIControl::SetHeight(m_texture.GetHeight());
}

void CGUIImage::SetPosition(float posX, float posY)
{
	m_texture.SetPosition(posX, posY);
	CGUIControl::SetPosition(posX, posY);
}

void CGUIImage::SetInfo(const CGUIInfoLabel &info)
{
	m_info = info;
	
	// A constant image never needs updating
	if (m_info.IsConstant())
		m_texture.SetFileName(m_info.GetLabel(0));
}

unsigned char CGUIImage::GetFadeLevel(unsigned int time) const
{
	float amount = (float)time / m_crossFadeTime;
	
	// We want a semi-transparent image, so we need to use a more complicated
	// fade technique.  Assuming a black background (not generally true, but still...)
	// we have
	// b(t) = [a - b(1-t)*a] / a*(1-b(1-t)*a),
	// where a = alpha, and b(t):[0,1] -> [0,1] is the blend function.
	// solving, we get
	// b(t) = [1 - (1-a)^t] / a
	const float alpha = 0.7f;
	return (unsigned char)(255.0f * (1 - pow(1-alpha, amount))/alpha);
}
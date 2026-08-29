#include "pictures/SlideShowPicture.h"
#include <math.h>
#include "guilib/GUIInfoManager.h"
#include "AdvancedSettings.h"
#include "GUISettings.h"
#include "Settings.h"
#include "guilib/GraphicContext.h"
#include "utils/Log.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define IMMEDIATE_TRANSISTION_TIME          20

#define PICTURE_MOVE_AMOUNT              0.02f
#define PICTURE_MOVE_AMOUNT_ANALOG       0.01f
#define PICTURE_VIEW_BOX_COLOR      0xffffff00 // YELLOW
#define PICTURE_VIEW_BOX_BACKGROUND 0xff000000 // BLACK

#define FPS                                 25

static float zoomamount[10] = { 1.0f, 1.2f, 1.5f, 2.0f, 2.8f, 4.0f, 6.0f, 9.0f, 13.5f, 20.0f };

// Shared shaders for drawing the slideshow quads
namespace SlideShowShaders
{
	// Vertex shader - screen space.  The slideshow geometry is already in
	// final screen pixels (equivalent of the original's D3DFVF_XYZRHW
	// pre-transformed vertices), so it must NOT be put through the GUI's
	// final matrix (which scales 720x576 skin space to the display).
	// Xenon has no pretransformed-vertex path, so convert pixels to
	// normalized device coordinates here.  c0 = (screenWidth, screenHeight)
	const char* g_strVertexShader =
		" float4 screenSize : register(c0);               "
		"                                               "
		" struct VS_IN                                  "
		" {                                             "
		"     float4 ObjPos   : POSITION;               "
		"     float4 Col      : COLOR0;                 "
		"     float2 TexCoord : TEXCOORD0;              "
		" };                                            "
		"                                               "
		" struct VS_OUT                                 "
		" {                                             "
		"     float4 ProjPos  : POSITION;               "
		"     float4 Col      : COLOR0;                 "
		"     float2 TexCoord : TEXCOORD0;              "
		" };                                            "
		"                                               "
		" VS_OUT main( VS_IN In )                       "
		" {                                             "
		"     VS_OUT Out;                               "
		"     Out.ProjPos = float4( In.ObjPos.x / screenSize.x * 2.0 - 1.0, "
		"                           1.0 - In.ObjPos.y / screenSize.y * 2.0, "
		"                           In.ObjPos.z, 1.0 ); "
		"     Out.Col = In.Col;                         "
		"     Out.TexCoord = In.TexCoord;               "
		"     return Out;                               "
		" }                                             ";

	// Pixel shader - texture modulated with the diffuse (fade) colour
	const char* g_strPixelShader =
		" sampler2D InputTexture : register(S0);        "
		"                                               "
		" float4 main(float2 uv : TEXCOORD0,            "
		"             float4 diffuse : COLOR0) : COLOR  "
		" {                                             "
		"    float4 color = tex2D( InputTexture, uv);   "
		"    color *= diffuse;                          "
		"    return color;                              "
		" }                                             ";

	// Pixel shader - diffuse colour only (zoom view box + black backing)
	const char* g_strPixelShaderPlain =
		" float4 main(float4 diffuse : COLOR0) : COLOR  "
		" {                                             "
		"    return diffuse;                            "
		" }                                             ";
}

IDirect3DVertexDeclaration9* CSlideShowPic::s_pVertexDecl = NULL;
IDirect3DVertexShader9*      CSlideShowPic::s_pVertexShader = NULL;
IDirect3DPixelShader9*       CSlideShowPic::s_pPixelShader = NULL;
IDirect3DPixelShader9*       CSlideShowPic::s_pPixelShaderPlain = NULL;
bool                         CSlideShowPic::s_bShadersAllocated = false;

CSlideShowPic::CSlideShowPic()
{
	m_pImage = NULL;
	m_bIsLoaded = false;
	m_bIsFinished = false;
	m_bDrawNextImage = false;
	m_bTransistionImmediately = false;
}

CSlideShowPic::~CSlideShowPic()
{
	Close();
}

void CSlideShowPic::Close()
{
	CSingleLock lock(m_textureAccess);
	if (m_pImage)
	{
		m_pImage->Release();
		m_pImage = NULL;
	}
	m_bIsLoaded = false;
	m_bIsFinished = false;
	m_bDrawNextImage = false;
	m_bTransistionImmediately = false;
}

void CSlideShowPic::SetTexture(int iSlideNumber, LPDIRECT3DTEXTURE9 pTexture, int iWidth, int iHeight, int iRotate, DISPLAY_EFFECT dispEffect, TRANSISTION_EFFECT transEffect)
{
	CSingleLock lock(m_textureAccess);
	Close();
	m_bPause = false;
	m_bNoEffect = false;
	m_bTransistionImmediately = false;
	m_iSlideNumber = iSlideNumber;
	m_pImage = pTexture;
	m_fWidth = (float)iWidth;
	m_fHeight = (float)iHeight;

	// Work out the maximum texture coordinates (the texture may be padded,
	// e.g. to a multiple of 4 or to power-of-two dimensions on the 360)
	m_fMaxU = 1.0f;
	m_fMaxV = 1.0f;

	if (pTexture)
	{
		D3DSURFACE_DESC desc;
		if (D3D_OK == pTexture->GetLevelDesc(0, &desc))
		{
			if (desc.Width > 0 && (unsigned)iWidth <= desc.Width)
				m_fMaxU = (float)iWidth / (float)desc.Width;
			if (desc.Height > 0 && (unsigned)iHeight <= desc.Height)
				m_fMaxV = (float)iHeight / (float)desc.Height;
		}
	}

	// Reset our counter
	m_iCounter = 0;
	// Initialize our transistion effect
	m_transistionStart.type = transEffect;
	m_transistionStart.start = 0;
	// The +1's make sure it actually occurs
	float fadeTime = (std::min)(0.2f*(float)g_guiSettings.GetInt("slideshow.staytime"), 3.0f);
	m_transistionStart.length = (int)(g_infoManager.GetFPS() * fadeTime); // transition time in frames
	m_transistionEnd.type = transEffect;
	m_transistionEnd.start = m_transistionStart.length + max((int)(g_infoManager.GetFPS() * g_guiSettings.GetInt("slideshow.staytime")), 1);
	m_transistionEnd.length = m_transistionStart.length;
	CLog::Log(LOGDEBUG,"Duration: %i (transistion out length %i)", m_transistionEnd.start, m_transistionEnd.length);
	m_transistionTemp.type = TRANSISTION_NONE;
	m_fTransistionAngle = 0;
	m_fTransistionZoom = 0;
	m_fAngle = 0;

	if (iRotate == 8)
	{
		// rotate to 270 degrees
		m_fAngle = 3.0f;
	}

	if (iRotate == 6)
	{
		// rotate to 90 degrees
		m_fAngle = 1.0f;
	}

	m_fZoomAmount = 1;
	m_fZoomLeft = 0;
	m_fZoomTop = 0;
	m_iTotalFrames = m_transistionStart.length + m_transistionEnd.length + max(((int)g_infoManager.GetFPS() * g_guiSettings.GetInt("slideshow.staytime")), 1);
	
	// Initialize our display effect
	if (dispEffect == EFFECT_RANDOM)
		m_displayEffect = (DISPLAY_EFFECT)((rand() % (EFFECT_RANDOM - 1)) + 1);
	else
		m_displayEffect = dispEffect;

	m_fPosX = m_fPosY = 0.0f;
	m_fPosZ = 1.0f;
	m_fVelocityX = m_fVelocityY = m_fVelocityZ = 0.0f;

	if (m_displayEffect == EFFECT_FLOAT)
	{
		// Calculate start and end positions
		// choose a random direction
		float angle = (rand() % 1000) / 1000.0f * 2 * (float)M_PI;
		m_fPosX = cos(angle) * g_advancedSettings.m_slideshowPanAmount * m_iTotalFrames * 0.00005f;
		m_fPosY = sin(angle) * g_advancedSettings.m_slideshowPanAmount * m_iTotalFrames * 0.00005f;
		m_fVelocityX = -m_fPosX * 2.0f / m_iTotalFrames;
		m_fVelocityY = -m_fPosY * 2.0f / m_iTotalFrames;
	}
	else if (m_displayEffect == EFFECT_ZOOM)
	{
		m_fPosZ = 1.0f;
		m_fVelocityZ = 0.0001f * g_advancedSettings.m_slideshowZoomAmount;
	}

	m_bIsFinished = false;
	m_bDrawNextImage = false;
	m_bIsLoaded = true;

	return;
}

void CSlideShowPic::SetOriginalSize(int iOriginalWidth, int iOriginalHeight, bool bFullSize)
{
	m_iOriginalWidth = iOriginalWidth;
	m_iOriginalHeight = iOriginalHeight;
	m_bFullSize = bFullSize;
}

int CSlideShowPic::GetOriginalWidth()
{
	int iAngle = (int)(m_fAngle + 0.4f);

	if (iAngle % 2)
		return m_iOriginalHeight;
	else
		return m_iOriginalWidth;
}

int CSlideShowPic::GetOriginalHeight()
{
	int iAngle = (int)(m_fAngle + 0.4f);

	if (iAngle % 2)
		return m_iOriginalWidth;
	else
		return m_iOriginalHeight;
}

void CSlideShowPic::UpdateTexture(IDirect3DTexture9 *pTexture, int iWidth, int iHeight)
{
	CSingleLock lock(m_textureAccess);

	if (m_pImage)
	{
		m_pImage->Release();
	}

	m_pImage = pTexture;
	m_fWidth = (float)iWidth;
	m_fHeight = (float)iHeight;
	m_fMaxU = 1.0f;
	m_fMaxV = 1.0f;

	if (pTexture)
	{
		D3DSURFACE_DESC desc;
		if (D3D_OK == pTexture->GetLevelDesc(0, &desc))
		{
			if (desc.Width > 0 && (unsigned)iWidth <= desc.Width)
				m_fMaxU = (float)iWidth / (float)desc.Width;
			if (desc.Height > 0 && (unsigned)iHeight <= desc.Height)
				m_fMaxV = (float)iHeight / (float)desc.Height;
		}
	}
}

void CSlideShowPic::Process()
{
	if (!m_pImage || !m_bIsLoaded || m_bIsFinished) return ;

	if (m_iCounter <= m_transistionStart.length)
	{
		// do start transistion
		if (m_transistionStart.type == CROSSFADE)
		{
			// fade in at 1x speed
			m_alpha = (color_t)((float)m_iCounter / (float)m_transistionStart.length * 255.0f);
		}
		else if (m_transistionStart.type == FADEIN_FADEOUT)
		{
			// fade in at 2x speed, then keep solid
			m_alpha = (color_t)((float)m_iCounter / (float)m_transistionStart.length * 255.0f * 2);
			if (m_alpha > 255) m_alpha = 255;
		}
		else // m_transistionEffect == TRANSISTION_NONE
		{
			m_alpha = 0xFF; // opaque
		}
	}

	bool bPaused = m_bPause | (m_fZoomAmount != 1.0f);

	// Check if we're doing a temporary effect (such as rotate + zoom)
	if (m_transistionTemp.type != TRANSISTION_NONE)
	{
		bPaused = true;
		if (m_iCounter >= m_transistionTemp.start)
		{
			if (m_iCounter >= m_transistionTemp.start + m_transistionTemp.length)
			{
				// we're finished this transistion
				if (m_transistionTemp.type == TRANSISTION_ZOOM)
				{
					// correct for any introduced inaccuracies.
					int i;

					for (i = 0; i < 10; i++)
						if (fabs(m_fZoomAmount - zoomamount[i]) < 0.01*zoomamount[i])
							break;

					m_fZoomAmount = zoomamount[i];
					m_bNoEffect = (m_fZoomAmount != 1.0f); // turn effect rendering back on.
				}

				if (m_transistionTemp.type == TRANSISTION_ROTATE)
				{
					// round to nearest integer for accuracy purposes
					m_fAngle = floor(m_fAngle + 0.4f);
				}
				m_transistionTemp.type = TRANSISTION_NONE;
			}
			else
			{
				if (m_transistionTemp.type == TRANSISTION_ROTATE)
				{
					m_fAngle += m_fTransistionAngle;
				}

				if (m_transistionTemp.type == TRANSISTION_ZOOM)
				{
					m_fZoomAmount += m_fTransistionZoom;
				}
			}
		}
	}

	// now just display
	if (!m_bNoEffect && !bPaused)
	{
		if (m_displayEffect == EFFECT_FLOAT)
		{
			m_fPosX += m_fVelocityX;
			m_fPosY += m_fVelocityY;
			float fMoveAmount = g_advancedSettings.m_slideshowPanAmount * m_iTotalFrames * 0.0001f;

			if (m_fPosX > fMoveAmount)
			{
				m_fPosX = fMoveAmount;
				m_fVelocityX = -m_fVelocityX;
			}
			if (m_fPosX < -fMoveAmount)
			{
				m_fPosX = -fMoveAmount;
				m_fVelocityX = -m_fVelocityX;
			}
			if (m_fPosY > fMoveAmount)
			{
				m_fPosY = fMoveAmount;
				m_fVelocityY = -m_fVelocityY;
			}
			if (m_fPosY < -fMoveAmount)
			{
				m_fPosY = -fMoveAmount;
				m_fVelocityY = -m_fVelocityY;
			}
		}
		else if (m_displayEffect == EFFECT_ZOOM)
		{
			m_fPosZ += m_fVelocityZ;
		}
	}

	if (m_displayEffect != EFFECT_NO_TIMEOUT && bPaused && !m_bTransistionImmediately)
	{
		// paused - increment the last transistion start time
		m_transistionEnd.start++;
	}

	if (m_iCounter >= m_transistionEnd.start)
	{
		// do end transistion
		m_bDrawNextImage = true;

		if (m_transistionEnd.type == CROSSFADE)
		{
			// fade out at 1x speed
			m_alpha = 255 - (color_t)((float)(m_iCounter - m_transistionEnd.start) / (float)m_transistionEnd.length * 255.0f);
		}
		else if (m_transistionEnd.type == FADEIN_FADEOUT)
		{
			// keep solid, then fade out at 2x speed
			m_alpha = (color_t)((float)(m_transistionEnd.length - m_iCounter + m_transistionEnd.start) / (float)m_transistionEnd.length * 255.0f * 2);
			if (m_alpha > 255) m_alpha = 255;
		}
		else // m_transistionEffect == TRANSISTION_NONE
		{
			m_alpha = 0xFF; // opaque
		}
	}

	if (m_displayEffect != EFFECT_NO_TIMEOUT || m_iCounter < m_transistionStart.length || m_iCounter >= m_transistionEnd.start || (m_iCounter >= m_transistionTemp.start && m_iCounter < m_transistionTemp.start + m_transistionTemp.length))
	{
		m_iCounter++;
	}

	if (m_iCounter > m_transistionEnd.start + m_transistionEnd.length)
		m_bIsFinished = true;
}

void CSlideShowPic::Keep()
{
	// This is called if we need to keep the current pic on screen
	// to wait for the next pic to load
	if (!m_bDrawNextImage) return ; // don't need to keep pic

	// Hold off the start of the next frame
	m_transistionEnd.start = m_iCounter;
}

bool CSlideShowPic::StartTransistion()
{
	// This is called if we need to start transistioning immediately to the new picture
	if (m_bDrawNextImage) return false; // don't need to do anything as we are already transistioning

	// Decrease the number of display frame
	m_transistionEnd.start = m_iCounter;
	m_bTransistionImmediately = true;

	return true;
}

void CSlideShowPic::Pause(bool bPause)
{
	if (!m_bDrawNextImage)
		m_bPause = bPause;
}

void CSlideShowPic::SetInSlideshow(bool slideshow)
{
	if (slideshow && m_displayEffect == EFFECT_NO_TIMEOUT)
		m_displayEffect = EFFECT_NONE;
}

int CSlideShowPic::GetTransistionTime(int iType) const
{
	if (iType == 0) // Start transistion
		return m_transistionStart.length;
	else // iType == 1 // End transistion
		return m_transistionEnd.length;
}

void CSlideShowPic::SetTransistionTime(int iType, int iTime)
{
	if (iType == 0) // Start transistion
		m_transistionStart.length = iTime;
	else // iType == 1 // end transistion
		m_transistionEnd.length = iTime;
}

void CSlideShowPic::Rotate(int iRotate)
{
	if (m_bDrawNextImage) return ;
	if (m_transistionTemp.type == TRANSISTION_ZOOM) return ;

	m_transistionTemp.type = TRANSISTION_ROTATE;
	m_transistionTemp.start = m_iCounter;
	m_transistionTemp.length = IMMEDIATE_TRANSISTION_TIME;
	m_fTransistionAngle = (float)(iRotate - m_fAngle) / (float)m_transistionTemp.length;

	// Reset the timer
	m_transistionEnd.start = m_iCounter + m_transistionStart.length + ((int)g_infoManager.GetFPS() * g_guiSettings.GetInt("slideshow.staytime"));
}

void CSlideShowPic::Zoom(int iZoom, bool immediate /*= false*/)
{
	if (m_bDrawNextImage) return ;
	if (m_transistionTemp.type == TRANSISTION_ROTATE) return ;
	
	if (immediate)
	{
		m_fZoomAmount = zoomamount[iZoom - 1];
		return;
	}

	m_transistionTemp.type = TRANSISTION_ZOOM;
	m_transistionTemp.start = m_iCounter;
	m_transistionTemp.length = IMMEDIATE_TRANSISTION_TIME;
	m_fTransistionZoom = (float)(zoomamount[iZoom - 1] - m_fZoomAmount) / (float)m_transistionTemp.length;

	// Reset the timer
	m_transistionEnd.start = m_iCounter + m_transistionStart.length + ((int)g_infoManager.GetFPS() * g_guiSettings.GetInt("slideshow.staytime"));
	
	// Turn off the render effects until we're back down to normal zoom
	m_bNoEffect = true;
}

void CSlideShowPic::Move(float fDeltaX, float fDeltaY)
{
	m_fZoomLeft += fDeltaX;
	m_fZoomTop += fDeltaY;

	// Reset the timer
	// m_transistionEnd.start = m_iCounter + m_transistionStart.length + ((int)g_infoManager.GetFPS() * g_guiSettings.GetInt("slideshow.staytime"));
}

void CSlideShowPic::Render()
{
	CSingleLock lock(m_textureAccess);
	if (!m_pImage || !m_bIsLoaded || m_bIsFinished) return ;

	// Update the image
	Process();

	// Calculate where we should render (and how large it should be)
	// calculate aspect ratio correction factor
	RESOLUTION iRes = g_graphicsContext.GetVideoResolution();
	float fOffsetX = (float)g_settings.m_ResInfo[iRes].Overscan.left;
	float fOffsetY = (float)g_settings.m_ResInfo[iRes].Overscan.top;
	float fScreenWidth = (float)g_settings.m_ResInfo[iRes].Overscan.right - g_settings.m_ResInfo[iRes].Overscan.left;
	float fScreenHeight = (float)g_settings.m_ResInfo[iRes].Overscan.bottom - g_settings.m_ResInfo[iRes].Overscan.top;

	float fPixelRatio = g_settings.m_ResInfo[iRes].fPixelRatio;

	// Rotate the image as needed
	float x[4];
	float y[4];
	float si = (float)sin(m_fAngle * M_PI * 0.5);
	float co = (float)cos(m_fAngle * M_PI * 0.5);
	x[0] = -m_fWidth * co + m_fHeight * si;
	y[0] = -m_fWidth * si - m_fHeight * co;
	x[1] = m_fWidth * co + m_fHeight * si;
	y[1] = m_fWidth * si - m_fHeight * co;
	x[2] = m_fWidth * co - m_fHeight * si;
	y[2] = m_fWidth * si + m_fHeight * co;
	x[3] = -m_fWidth * co - m_fHeight * si;
	y[3] = -m_fWidth * si + m_fHeight * co;

	// Calculate our scale amounts
	float fSourceAR = m_fWidth / m_fHeight;
	float fSourceInvAR = 1 / fSourceAR;
	float fAR = si * si * (fSourceInvAR - fSourceAR) + fSourceAR;

	float fScaleNorm = fScreenWidth / m_fWidth;
	float fScaleInv = fScreenWidth / m_fHeight;

	bool bFillScreen = false;
	float fComp = 1.0f + 0.01f * g_advancedSettings.m_slideshowBlackBarCompensation;
	float fScreenRatio = fScreenWidth / fScreenHeight * fPixelRatio;

	// Work out if we should be compensating the zoom to minimize blackbars
	// we should compute this based on the % of black bars on screen perhaps??
	// TODO: change m_displayEffect != EFFECT_NO_TIMEOUT to whether we're running the slideshow
	if (m_displayEffect != EFFECT_NO_TIMEOUT && fScreenRatio < fSourceAR * fComp && fSourceAR < fScreenRatio * fComp)
		bFillScreen = true;
	if ((!bFillScreen && fScreenWidth*fPixelRatio > fScreenHeight*fSourceAR) || (bFillScreen && fScreenWidth*fPixelRatio < fScreenHeight*fSourceAR))
		fScaleNorm = fScreenHeight / (m_fHeight * fPixelRatio);
	bFillScreen = false;
	if (m_displayEffect != EFFECT_NO_TIMEOUT && fScreenRatio < fSourceInvAR * fComp && fSourceInvAR < fScreenRatio * fComp)
		bFillScreen = true;
	if ((!bFillScreen && fScreenWidth*fPixelRatio > fScreenHeight*fSourceInvAR) || (bFillScreen && fScreenWidth*fPixelRatio < fScreenHeight*fSourceInvAR))
		fScaleInv = fScreenHeight / (m_fWidth * fPixelRatio);

	float fScale = si * si * (fScaleInv - fScaleNorm) + fScaleNorm;

	// Scale if we need to due to the effect we're using
	if (m_displayEffect == EFFECT_FLOAT)
		fScale *= (1.0f + g_advancedSettings.m_slideshowPanAmount * m_iTotalFrames * 0.0001f);
	if (m_displayEffect == EFFECT_ZOOM)
		fScale *= m_fPosZ;

	// Zoom image
	fScale *= m_fZoomAmount;

	// Calculate the resultant coordinates
	for (int i = 0; i < 4; i++)
	{
		x[i] *= fScale * 0.5f; // as the offsets x[] and y[] are from center
		y[i] *= fScale * fPixelRatio * 0.5f;
		// center it
		x[i] += 0.5f * fScreenWidth + fOffsetX;
		y[i] += 0.5f * fScreenHeight + fOffsetY;
	}

	// Shift if we're zooming
	if (m_fZoomAmount > 1)
	{
		float minx = x[0];
		float maxx = x[0];
		float miny = y[0];
		float maxy = y[0];

		for (int i = 1; i < 4; i++)
		{
			if (x[i] < minx) minx = x[i];
			if (x[i] > maxx) maxx = x[i];
			if (y[i] < miny) miny = y[i];
			if (y[i] > maxy) maxy = y[i];
		}

		float w = maxx - minx;
		float h = maxy - miny;

		if (w >= fScreenWidth)
		{
			// must have no black bars
			if (minx + m_fZoomLeft*w > fOffsetX)
				m_fZoomLeft = (fOffsetX - minx) / w;
			if (maxx + m_fZoomLeft*w < fOffsetX + fScreenWidth)
				m_fZoomLeft = (fScreenWidth + fOffsetX - maxx) / w;
			for (int i = 0; i < 4; i++)
				x[i] += w * m_fZoomLeft;
		}

		if (h >= fScreenHeight)
		{
			// must have no black bars
			if (miny + m_fZoomTop*h > fOffsetY)
				m_fZoomTop = (fOffsetY - miny) / h;
			if (maxy + m_fZoomTop*h < fOffsetY + fScreenHeight)
				m_fZoomTop = (fScreenHeight + fOffsetY - maxy) / h;
			for (int i = 0; i < 4; i++)
				y[i] += m_fZoomTop * h;
		}
	}

	// add offset from display effects
	for (int i = 0; i < 4; i++)
	{
		x[i] += m_fPosX * m_fWidth * fScale;
		y[i] += m_fPosY * m_fHeight * fScale;
	}

	// and render
	Render(x, y, m_pImage, (m_alpha << 24) | 0xFFFFFF);

	// Now render the image in the top right corner if we're zooming
	if (m_fZoomAmount == 1 || m_bIsComic) return ;

	float sx[4], sy[4];
	sx[0] = -m_fWidth * co + m_fHeight * si;
	sy[0] = -m_fWidth * si - m_fHeight * co;
	sx[1] = m_fWidth * co + m_fHeight * si;
	sy[1] = m_fWidth * si - m_fHeight * co;
	sx[2] = m_fWidth * co - m_fHeight * si;
	sy[2] = m_fWidth * si + m_fHeight * co;
	sx[3] = -m_fWidth * co - m_fHeight * si;
	sy[3] = -m_fWidth * si + m_fHeight * co;

	// Convert to the appropriate scale
	float fSmallArea = fScreenWidth * fScreenHeight / 50;
	float fSmallWidth = sqrt(fSmallArea * fAR / fPixelRatio); // fAR*height = width, so total area*far = width*width
	float fSmallHeight = fSmallArea / fSmallWidth;
	float fSmallX = fOffsetX + fScreenWidth * 0.95f - fSmallWidth * 0.5f;
	float fSmallY = fOffsetY + fScreenHeight * 0.05f + fSmallHeight * 0.5f;
	fScaleNorm = fSmallWidth / m_fWidth;
	fScaleInv = fSmallWidth / m_fHeight;
	fScale = si * si * (fScaleInv - fScaleNorm) + fScaleNorm;

	for (int i = 0; i < 4; i++)
	{
		sx[i] *= fScale * 0.5f;
		sy[i] *= fScale * fPixelRatio * 0.5f;
	}

	// calculate a black border
	float bx[4];
	float by[4];

	for (int i = 0; i < 4; i++)
	{
		if (sx[i] > 0)
			bx[i] = sx[i] + 1;
		else
			bx[i] = sx[i] - 1;
		if (sy[i] > 0)
			by[i] = sy[i] + 1;
		else
			by[i] = sy[i] - 1;
		sx[i] += fSmallX;
		sy[i] += fSmallY;
		bx[i] += fSmallX;
		by[i] += fSmallY;
	}

	fSmallX -= fSmallWidth * 0.5f;
	fSmallY -= fSmallHeight * 0.5f;

	Render(bx, by, NULL, PICTURE_VIEW_BOX_BACKGROUND);
	Render(sx, sy, m_pImage, 0xFFFFFFFF);

	// Now we must render the wireframe image of the view window
	// work out the direction of the top of pic vector
	float scale;
	if (fabs(x[1] - x[0]) > fabs(x[3] - x[0]))
		scale = (sx[1] - sx[0]) / (x[1] - x[0]);
	else
		scale = (sx[3] - sx[0]) / (x[3] - x[0]);
	float ox[4];
	float oy[4];
	ox[0] = (fOffsetX - x[0]) * scale + sx[0];
	oy[0] = (fOffsetY - y[0]) * scale + sy[0];
	ox[1] = (fOffsetX + fScreenWidth - x[0]) * scale + sx[0];
	oy[1] = (fOffsetY - y[0]) * scale + sy[0];
	ox[2] = (fOffsetX + fScreenWidth - x[0]) * scale + sx[0];
	oy[2] = (fOffsetY + fScreenHeight - y[0]) * scale + sy[0];
	ox[3] = (fOffsetX - x[0]) * scale + sx[0];
	oy[3] = (fOffsetY + fScreenHeight - y[0]) * scale + sy[0];

	// Crop to within the range of our piccy
	for (int i = 0; i < 4; i++)
	{
		if (ox[i] < fSmallX) ox[i] = fSmallX;
		if (ox[i] > fSmallX + fSmallWidth) ox[i] = fSmallX + fSmallWidth;
		if (oy[i] < fSmallY) oy[i] = fSmallY;
		if (oy[i] > fSmallY + fSmallHeight) oy[i] = fSmallY + fSmallHeight;
	}

	Render(ox, oy, NULL, PICTURE_VIEW_BOX_COLOR, D3DFILL_WIREFRAME);
}

void CSlideShowPic::EnsureShaders(LPDIRECT3DDEVICE9 pDevice)
{
	if (s_bShadersAllocated || !pDevice)
		return;

	ID3DXBuffer* pCode = NULL;
	ID3DXBuffer* pErrorMsg = NULL;

	D3DXCompileShader(SlideShowShaders::g_strVertexShader,
										(UINT)strlen(SlideShowShaders::g_strVertexShader),
										NULL, NULL, "main", "vs_2_0", 0,
										&pCode, &pErrorMsg, NULL);
	if (pCode)
		pDevice->CreateVertexShader((DWORD*)pCode->GetBufferPointer(), &s_pVertexShader);
	if (pCode) pCode->Release();
	if (pErrorMsg) pErrorMsg->Release();

	pCode = NULL;
	D3DXCompileShader(SlideShowShaders::g_strPixelShader,
										(UINT)strlen(SlideShowShaders::g_strPixelShader),
										NULL, NULL, "main", "ps_2_0", 0,
										&pCode, &pErrorMsg, NULL);
	if (pCode)
		pDevice->CreatePixelShader((DWORD*)pCode->GetBufferPointer(), &s_pPixelShader);
	if (pCode) pCode->Release();
	if (pErrorMsg) pErrorMsg->Release();

	pCode = NULL;
	D3DXCompileShader(SlideShowShaders::g_strPixelShaderPlain,
										(UINT)strlen(SlideShowShaders::g_strPixelShaderPlain),
										NULL, NULL, "main", "ps_2_0", 0,
										&pCode, &pErrorMsg, NULL);
	if (pCode)
		pDevice->CreatePixelShader((DWORD*)pCode->GetBufferPointer(), &s_pPixelShaderPlain);
	if (pCode) pCode->Release();
	if (pErrorMsg) pErrorMsg->Release();

	static const D3DVERTEXELEMENT9 VertexElements[] =
	{
		// Matches struct VERTEX { D3DXVECTOR4 p; D3DCOLOR col; FLOAT tu, tv; }
		{ 0,  0, D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 16, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 },
		{ 0, 20, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};
	pDevice->CreateVertexDeclaration(VertexElements, &s_pVertexDecl);

	s_bShadersAllocated = true;
}

void CSlideShowPic::Render(float *x, float *y, IDirect3DTexture9 *pTexture, color_t color, _D3DFILLMODE fillmode)
{
	LPDIRECT3DDEVICE9 pDevice = g_graphicsContext.Get3DDevice();
	if (!pDevice)
		return;

	EnsureShaders(pDevice);
	if (!s_pVertexShader || !s_pVertexDecl || !s_pPixelShader || !s_pPixelShaderPlain)
		return;

	VERTEX vertex[4];

	// Normalized texture coordinates over the (possibly padded) texture
	float maxU = pTexture ? m_fMaxU : 1.0f;
	float maxV = pTexture ? m_fMaxV : 1.0f;

	for (int i = 0; i < 4; i++)
	{
		// D3D aligns to half pixel boundaries
		vertex[i].p = D3DXVECTOR4(x[i] - 0.5f, y[i] - 0.5f, 0, 1.0f);
		vertex[i].tu = 0;
		vertex[i].tv = 0;
		vertex[i].col = color;
	}
	vertex[1].tu = maxU;
	vertex[2].tu = maxU;
	vertex[2].tv = maxV;
	vertex[3].tv = maxV;

	// Set state to render the image
	if (pTexture) pDevice->SetTexture( 0, pTexture );
	pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
	pDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	pDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	pDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
	// D3DRS_FOGENABLE removed - no fog support on Xenon
	pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	pDevice->SetVertexDeclaration( s_pVertexDecl );
	pDevice->SetVertexShader( s_pVertexShader );
	pDevice->SetPixelShader( pTexture ? s_pPixelShader : s_pPixelShaderPlain );
	// Screen dimensions for the pixel -> NDC conversion
	float fScreenSize[4] = { (float)g_graphicsContext.GetWidth(), (float)g_graphicsContext.GetHeight(), 0.0f, 0.0f };
	pDevice->SetVertexShaderConstantF( 0, fScreenSize, 1 );

	if (fillmode == D3DFILL_WIREFRAME)
	{
		// The 360 supports no fillmode rendering - Draw the box as line segments
		// edges: 0-1, 1-2, 2-3, 3-0
		VERTEX lines[8] = { vertex[0], vertex[1], vertex[1], vertex[2],
												vertex[2], vertex[3], vertex[3], vertex[0] };
		pDevice->DrawPrimitiveUP( D3DPT_LINELIST, 4, lines, sizeof(VERTEX) );
	}
	else
	{
		// two triangles (v0, v1, v2) and (v0, v2, v3)
		VERTEX tri[6] = { vertex[0], vertex[1], vertex[2],
											vertex[0], vertex[2], vertex[3] };
		pDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 2, tri, sizeof(VERTEX) );
	}
	if (pTexture) pDevice->SetTexture(0, NULL);
}

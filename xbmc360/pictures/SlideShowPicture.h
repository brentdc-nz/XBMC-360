#pragma once

#include "utils/SingleLock.h"
#include "utils/StdString.h"
#include <xtl.h>

typedef uint32_t color_t;

class CSlideShowPic
{
public:
	enum DISPLAY_EFFECT { EFFECT_NONE = 0, EFFECT_FLOAT, EFFECT_ZOOM, EFFECT_RANDOM, EFFECT_NO_TIMEOUT };
	enum TRANSISTION_EFFECT { TRANSISTION_NONE = 0, FADEIN_FADEOUT, CROSSFADE, TRANSISTION_ZOOM, TRANSISTION_ROTATE };

	struct TRANSISTION
	{
		TRANSISTION_EFFECT type;
		int start;
		int length;
	};

	CSlideShowPic();
	~CSlideShowPic();

	void SetTexture(int iSlideNumber, LPDIRECT3DTEXTURE9 pTexture, int iWidth, int iHeight, int iRotate, DISPLAY_EFFECT dispEffect = EFFECT_RANDOM, TRANSISTION_EFFECT transEffect = FADEIN_FADEOUT);
	void UpdateTexture(IDirect3DTexture9 *pTexture, int iWidth, int iHeight);
	bool IsLoaded() const { return m_bIsLoaded;};
	void UnLoad() {m_bIsLoaded = false;};
	void Render();
	void Close();
	bool IsFinished() const { return m_bIsFinished;};
	bool DrawNextImage() const { return m_bDrawNextImage;};

	int GetWidth() const { return (int)m_fWidth;};
	int GetHeight() const { return (int)m_fHeight;};

	void Keep();
	bool StartTransistion();
	int GetTransistionTime(int iType) const;
	void SetTransistionTime(int iType, int iTime);

	int SlideNumber() const { return m_iSlideNumber;};

	void Zoom(int iZoomAmount, bool immediate = false);
	void Rotate(int iRotateAmount);
	void Pause(bool bPause);
	void SetInSlideshow(bool slideshow);
	void SetOriginalSize(int iOriginalWidth, int iOriginalHeight, bool bFullSize);
	bool FullSize() const { return m_bFullSize;};
	int GetOriginalWidth();
	int GetOriginalHeight();

	void Move(float dX, float dY);
	float GetZoom() const { return m_fZoomAmount;};

	bool m_bIsComic;
private:
	void Process();
	void Render(float *x, float *y, IDirect3DTexture9 *pTexture, color_t color, _D3DFILLMODE fillmode = D3DFILL_SOLID );
	void EnsureShaders(LPDIRECT3DDEVICE9 pDevice);

	struct VERTEX
	{
		D3DXVECTOR4 p;
		D3DCOLOR col;
		FLOAT tu, tv;
	};

	int m_iOriginalWidth;
	int m_iOriginalHeight;
	int m_iSlideNumber;
	bool m_bIsLoaded;
	bool m_bIsFinished;
	bool m_bDrawNextImage;
	CStdString m_strFileName;
	IDirect3DTexture9* m_pImage;
	float m_fWidth;
	float m_fHeight;
	// Maximum texture coordinates (image size / texture size, handles
	// power-of-two padded textures on the 360)
	float m_fMaxU;
	float m_fMaxV;
	color_t m_alpha;
	// Stuff relative to middle position
	float m_fPosX;
	float m_fPosY;
	float m_fPosZ;
	float m_fVelocityX;
	float m_fVelocityY;
	float m_fVelocityZ;
	float m_fZoomAmount;
	float m_fZoomLeft;
	float m_fZoomTop;
	// Transistion and display effects
	DISPLAY_EFFECT m_displayEffect;
	TRANSISTION m_transistionStart;
	TRANSISTION m_transistionEnd;
	TRANSISTION m_transistionTemp; // Used for rotations + zooms
	float m_fAngle; // Angle (between 0 and 2pi to display the image)
	float m_fTransistionAngle;
	float m_fTransistionZoom;
	int m_iCounter;
	int m_iTotalFrames;
	bool m_bPause;
	bool m_bNoEffect;
	bool m_bFullSize;
	bool m_bTransistionImmediately;

	// Shared shaders for rendering the quad (Xenon has no fixed function
	// pipeline, modelled on CGUITextureD3D)
	static IDirect3DVertexDeclaration9* s_pVertexDecl;
	static IDirect3DVertexShader9*      s_pVertexShader;
	static IDirect3DPixelShader9*       s_pPixelShader;       // textured
	static IDirect3DPixelShader9*       s_pPixelShaderPlain;  // diffuse colour only
	static bool                         s_bShadersAllocated;

	CCriticalSection m_textureAccess;
};

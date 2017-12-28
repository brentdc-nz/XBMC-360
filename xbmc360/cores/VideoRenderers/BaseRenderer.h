#ifndef CBASERENDERER_H
#define CBASERENDERER_H

#include "..\..\utils\stdafx.h"

typedef struct RGB32Image_s
{
	BYTE *plane;
	unsigned int stride;
	unsigned int width;
	unsigned int height;
} RGB32Image_t;

class CBaseRenderer
{
public:
	CBaseRenderer() {};
	virtual ~CBaseRenderer() {};

	virtual void RenderUpdate(bool clear, DWORD flags = 0, DWORD alpha = 255) = 0;
	virtual bool PreInit() = 0;
	virtual void ManageDisplay() = 0;
	virtual bool Configure(int iWidth, int iHeight) = 0;
	virtual bool GetImage(RGB32Image_t *image) = 0;
	virtual void ReleaseImage() = 0;
	virtual void Render() = 0;
	virtual void FlipPage() = 0;
	virtual void PrepareDisplay() = 0;
	virtual void UnInit() = 0;

protected:
	int m_iPosX;
	int m_iPosY;
	int m_iWidth;
	int m_iHeight;
	
	LPDIRECT3DDEVICE9 m_pd3dDevice;

	bool m_initialized;

	int m_iSourceWidth;
	int m_iSourceHeight;

	int m_iScreenWidth;
	int m_iScreenHeight;

	int m_iActiveWidth;
	int m_iActiveHeight;

	int m_iActivePosX;
	int m_iActivePosY;
};

#endif //CBASERENDERER_H
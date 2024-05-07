#ifndef CBASERENDERER_H
#define CBASERENDERER_H

#include "..\..\utils\Stdafx.h"

#define MAX_PLANES 3
#define MAX_FIELDS 3

typedef struct YV12Image_s
{
	BYTE *   plane[MAX_PLANES];
	unsigned stride[MAX_PLANES];
	unsigned int width;
	unsigned int height;
} YV12Image;

class CBaseRenderer
{
public:
	CBaseRenderer() {};
	virtual ~CBaseRenderer() {};

	virtual void Update(bool bPauseDrawing) = 0;
	virtual void RenderUpdate(bool clear, DWORD flags = 0, DWORD alpha = 255) = 0;
	virtual bool PreInit() = 0;
	virtual void ManageDisplay() = 0;
	virtual bool Configure(int iWidth, int iHeight) = 0;
	virtual bool IsConfigured() = 0;
	virtual bool GetImage(YV12Image* image) = 0;
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

	bool m_bInitialized;
	bool m_bConfigured;
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
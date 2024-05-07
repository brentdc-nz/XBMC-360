#ifndef RGB_RENDERER_H
#define RGB_RENDERER_H

#include "BaseRenderer.h"
#include "utils\Stdafx.h"

class CRGBRenderer : public CBaseRenderer
{
public:
	CRGBRenderer(LPDIRECT3DDEVICE9 pDevice);
	virtual ~CRGBRenderer();

	virtual void Update(bool bPauseDrawing);
	virtual void RenderUpdate(bool clear, DWORD flags = 0, DWORD alpha = 255);
	virtual bool PreInit();
	virtual void ManageDisplay();
	virtual bool Configure(int iWidth, int iHeight);
	virtual bool IsConfigured() { return m_bConfigured; } 
	virtual bool GetImage(YV12Image *image);
	virtual void ReleaseImage();
	virtual void Render();
	virtual void PrepareDisplay();
	virtual void FlipPage();
	virtual void UnInit();

private:
	struct COLORVERTEX
	{
		float   Position[3];
		float   TexCoord[2];
	};

	bool m_bPrepared;

	IDirect3DVertexBuffer9*			m_pVB;           // Buffer to hold vertices
	IDirect3DVertexDeclaration9*	m_pVertexDecl;   // Vertex format decl
	IDirect3DVertexShader9*			m_pVertexShader; // Vertex Shader
	IDirect3DPixelShader9*			m_pPixelShader;  // Pixel Shader
	IDirect3DTexture9*				m_pFrameU;
	IDirect3DTexture9*				m_pFrameV;
	IDirect3DTexture9*				m_pFrameY;
};

#endif //RGB_RENDERER_H
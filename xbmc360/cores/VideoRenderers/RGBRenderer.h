#ifndef RGB_RENDERER_H
#define RGB_RENDERER_H

#include "BaseRenderer.h"
#include "..\..\Utils\stdafx.h"

class CRGBRenderer : public CBaseRenderer
{
public:
	CRGBRenderer(LPDIRECT3DDEVICE9 pDevice);
	virtual ~CRGBRenderer();

	virtual void RenderUpdate(bool clear, DWORD flags = 0, DWORD alpha = 255);
	virtual bool PreInit();
	virtual void ManageDisplay();
	virtual bool Configure(int iWidth, int iHeight);
	virtual bool GetImage(RGB32Image_t *image);
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
	LPDIRECT3DTEXTURE9				m_pTexture;

	D3DXMATRIX						m_matWorld;
	D3DXMATRIX						m_matProj;
	D3DXMATRIX						m_matView;
};

#endif //RGB_RENDERER_H
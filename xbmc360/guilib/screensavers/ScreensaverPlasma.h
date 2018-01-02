#ifndef H_GUILIB_SCREENSAVERPLASMA
#define H_GUILIB_SCREENSAVERPLASMA

#include "ScreensaverBase.h"
#include "..\..\utils\stdafx.h"

class CScreensaverPlasma : public CSceensaverBase
{
public:
	CScreensaverPlasma();
	~CScreensaverPlasma();

	virtual bool Initialize();
	virtual void Render();
	virtual bool Close();

private:
	void UpdateTexture();

	struct COLORVERTEX
	{
		float   Position[3];
		float   TexCoord[2];
	};

	IDirect3DVertexBuffer9*			m_pVB;           // Buffer to hold vertices
	IDirect3DVertexDeclaration9*	m_pVertexDecl;   // Vertex format decl
	IDirect3DVertexShader9*			m_pVertexShader; // Vertex Shader
	IDirect3DPixelShader9*			m_pPixelShader;  // Pixel Shader
	LPDIRECT3DTEXTURE9				m_pTexture;

	D3DXMATRIX						m_matWorld;
	D3DXMATRIX						m_matProj;
	D3DXMATRIX						m_matView;

	LPDIRECT3DDEVICE9 m_pd3dDevice;
	bool m_initialized;
	int m_iScreenWidth;
	int m_iScreenHeight;

	unsigned char m_r[256], m_g[256], m_b[256];
};

#endif //H_GUILIB_SCREENSAVERPLASMA

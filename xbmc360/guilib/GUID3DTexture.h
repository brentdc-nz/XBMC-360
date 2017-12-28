#ifndef GUILIB_GUID3DTEXTURE_H
#define GUILIB_GUID3DTEXTURE_H

#include "..\utils\stdafx.h"
#include "GUITexture.h"

#include <string>

class CGUID3DTexture : public CGUITextureBase
{
public:
	CGUID3DTexture(float posX, float posY, float width, float height, const CTextureInfo& texture);
	~CGUID3DTexture();
	void SetFileName(const CStdString &strFilename);

	bool AllocResources();
	bool FreeResources();
	void Render();
	
private:
	CStdString m_strFilename;
	bool m_initialized;

	LPDIRECT3DDEVICE9 m_pd3dDevice;

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
};

#endif //GUILIB_GUID3DTEXTURE_H
#ifndef GUILIB_GUITEXTURED3D_H
#define GUILIB_GUITEXTURED3D_H

#include "GUITexture.h"

#define CGUITexture CGUITextureD3D

class CGUITextureD3D : public CGUITextureBase
{
public:
	CGUITextureD3D(float posX, float posY, float width, float height, const CTextureInfo& texture);

protected:
	void Begin();
	void Draw(float *x, float *y, float *z, const CRect &texture, const CRect &diffuse, color_t color, int orientation);
	void End();

	virtual void Allocate();
	virtual void Free();

private:
	struct D3DCUSTOMVERTEX
	{
		float   Position[3];
		float   TexCoord[2];
	};

	LPDIRECT3DDEVICE9 m_pd3dDevice;

	IDirect3DVertexBuffer9*			m_pVB;           // Buffer to hold vertices
	IDirect3DVertexDeclaration9*	m_pVertexDecl;   // Vertex format decl
	IDirect3DVertexShader9*			m_pVertexShader; // Vertex Shader
	IDirect3DPixelShader9*			m_pPixelShader;  // Pixel Shader
};

#endif //GUILIB_GUITEXTURED3D_H
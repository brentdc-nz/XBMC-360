#ifndef GUILIB_GUITEXTURED3D_H
#define GUILIB_GUITEXTURED3D_H

#include "GUITexture.h"

#define CGUITexture CGUITextureD3D

class CGUITextureD3D : public CGUITextureBase
{
public:
	CGUITextureD3D(float posX, float posY, float width, float height, const CTextureInfo& texture);

	// Shared GPU resources (compiled once, used by all instances)
	static void AllocateShared(LPDIRECT3DDEVICE9 pDevice);
	static void FreeShared();

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
		float   TexCoord2[2];
	};

	LPDIRECT3DDEVICE9 m_pd3dDevice;

	// Shared static resources
	static IDirect3DVertexDeclaration9*	s_pVertexDecl;
	static IDirect3DVertexShader9*		s_pVertexShader;
	static IDirect3DPixelShader9*		s_pPixelShader;
	static IDirect3DPixelShader9*		s_pPixelShaderDiffuse;
	static bool							s_bSharedAllocated;
};

#endif //GUILIB_GUITEXTURED3D_H
#include <xtl.h>

// First Legacy Pixel Shader - #define WFVERTEX_FORMAT     (D3DFVF_XYZ | D3DFVF_DIFFUSE )
extern D3DVertexShader*       g_pVertexShader_Legacy1;
extern D3DPixelShader*        g_pPixelShader_Legacy1;

// Second Pixel Shader = #define SPRITEVERTEX_FORMAT (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0) )
extern D3DVertexShader*       g_pVertexShader_Legacy2;
extern D3DPixelShader*        g_pPixelShader_Legacy2;

//#define SPRITEVERTEX_FORMAT (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0) )
extern D3DVertexShader*       g_pVertexShader_Legacy3;
extern D3DPixelShader*        g_pPixelShader_Legacy3;

class CLegacyShaders
{
public:
	static void CreateShaders(D3DDevice* pD3dDevice);
	static void DeleteShaders();
};
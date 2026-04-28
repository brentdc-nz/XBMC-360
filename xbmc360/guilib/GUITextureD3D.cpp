#include "GUITextureD3D.h"
#include "GraphicContext.h"

namespace D3DTextureShaders
{
// Vertex shader - passes through both texture coordinate sets
const char* g_strVertexShader =
    " float4x4 matWVP : register(c0);              "
    "                                              "
    " struct VS_IN                                 "
    " {                                            "
    "     float4 ObjPos   : POSITION;              "
    "     float2 TexCoord : TEXCOORD0;             "
    "     float2 TexCoord2 : TEXCOORD1;            "
    " };                                           "
    "                                              "
    " struct VS_OUT                                "
    " {                                            "
    "     float4 ProjPos  : POSITION;              "
    "     float2 TexCoord : TEXCOORD0;             "
    "     float2 TexCoord2 : TEXCOORD1;            "
    " };                                           "
    "                                              "
    " VS_OUT main( VS_IN In )                      "
    " {                                            "
    "     VS_OUT Out;                              "
    "     Out.ProjPos = mul( matWVP, In.ObjPos );  "
    "     Out.TexCoord = float2(clamp(In.TexCoord.x, 0.0, 1.0), clamp(In.TexCoord.y, 0.0, 1.0)); "
    "     Out.TexCoord2 = In.TexCoord2;            "
    "     return Out;                              "
    " }                                            ";

// Pixel shader - single texture
const char* g_strPixelShader =
    " float InputAlpha : register(c10);             "
    " sampler2D InputTexture : register(S0);        "
    "                                               "
    " float4 main(float2 uv : TEXCOORD0) : COLOR    "
    " {                                             "
    "    float4 color = tex2D( InputTexture, uv.xy);"
    "    color[3] = InputAlpha * color[3];          "
    "    return color;                              "
    " }                                             ";

// Pixel shader - with diffuse texture (used for effects like mirror reflections)
const char* g_strPixelShaderDiffuse =
    " float InputAlpha : register(c10);             "
    " sampler2D InputTexture : register(S0);        "
    " sampler2D DiffuseTexture : register(S1);      "
    "                                               "
    " float4 main(float2 uv : TEXCOORD0,            "
    "             float2 uv2 : TEXCOORD1) : COLOR   "
    " {                                             "
    "    float4 color = tex2D( InputTexture, uv.xy);"
    "    float4 diff = tex2D( DiffuseTexture, uv2.xy);"
    "    color *= diff;                             "
    "    color[3] = InputAlpha * color[3];          "
    "    return color;                              "
    " }                                             ";
}

CGUITextureD3D::CGUITextureD3D(float posX, float posY, float width, float height, const CTextureInfo &texture)
: CGUITextureBase(posX, posY, width, height, texture)
{
}

// Static shared resource definitions
IDirect3DVertexDeclaration9* CGUITextureD3D::s_pVertexDecl = NULL;
IDirect3DVertexShader9*      CGUITextureD3D::s_pVertexShader = NULL;
IDirect3DPixelShader9*       CGUITextureD3D::s_pPixelShader = NULL;
IDirect3DPixelShader9*       CGUITextureD3D::s_pPixelShaderDiffuse = NULL;
bool                         CGUITextureD3D::s_bSharedAllocated = false;

void CGUITextureD3D::AllocateShared(LPDIRECT3DDEVICE9 pDevice)
{
	if (s_bSharedAllocated || !pDevice)
		return;

	// Compile vertex shader
	ID3DXBuffer* pVertexShaderCode = NULL;
	ID3DXBuffer* pVertexErrorMsg = NULL;

	D3DXCompileShader( D3DTextureShaders::g_strVertexShader,
                       ( UINT )strlen( D3DTextureShaders::g_strVertexShader ),
                       NULL, NULL, "main", "vs_2_0", 0,
                       &pVertexShaderCode, &pVertexErrorMsg, NULL );

	pDevice->CreateVertexShader( ( DWORD* )pVertexShaderCode->GetBufferPointer(), &s_pVertexShader );

	// Compile pixel shader
	ID3DXBuffer* pPixelShaderCode = NULL;
	ID3DXBuffer* pPixelErrorMsg = NULL;

	D3DXCompileShader( D3DTextureShaders::g_strPixelShader,
                       ( UINT )strlen( D3DTextureShaders::g_strPixelShader ),
                       NULL, NULL, "main", "ps_2_0", 0,
                       &pPixelShaderCode, &pPixelErrorMsg, NULL );

	pDevice->CreatePixelShader( ( DWORD* )pPixelShaderCode->GetBufferPointer(), &s_pPixelShader );

	// Compile diffuse pixel shader
	ID3DXBuffer* pPixelShaderDiffuseCode = NULL;
	ID3DXBuffer* pPixelDiffuseErrorMsg = NULL;

	D3DXCompileShader( D3DTextureShaders::g_strPixelShaderDiffuse,
                       ( UINT )strlen( D3DTextureShaders::g_strPixelShaderDiffuse ),
                       NULL, NULL, "main", "ps_2_0", 0,
                       &pPixelShaderDiffuseCode, &pPixelDiffuseErrorMsg, NULL );

	pDevice->CreatePixelShader( ( DWORD* )pPixelShaderDiffuseCode->GetBufferPointer(), &s_pPixelShaderDiffuse );

	// Create shared vertex declaration
	static const D3DVERTEXELEMENT9 VertexElements[] =
	{
		{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, 20, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
		D3DDECL_END()
	};

	pDevice->CreateVertexDeclaration( VertexElements, &s_pVertexDecl );

	// Release compile buffers
	if (pVertexShaderCode) pVertexShaderCode->Release();
	if (pVertexErrorMsg) pVertexErrorMsg->Release();
	if (pPixelShaderCode) pPixelShaderCode->Release();
	if (pPixelErrorMsg) pPixelErrorMsg->Release();
	if (pPixelShaderDiffuseCode) pPixelShaderDiffuseCode->Release();
	if (pPixelDiffuseErrorMsg) pPixelDiffuseErrorMsg->Release();

	s_bSharedAllocated = true;
}

void CGUITextureD3D::FreeShared()
{
	if (!s_bSharedAllocated)
		return;

	if (s_pVertexShader) { s_pVertexShader->Release(); s_pVertexShader = NULL; }
	if (s_pVertexDecl) { s_pVertexDecl->Release(); s_pVertexDecl = NULL; }
	if (s_pPixelShader) { s_pPixelShader->Release(); s_pPixelShader = NULL; }
	if (s_pPixelShaderDiffuse) { s_pPixelShaderDiffuse->Release(); s_pPixelShaderDiffuse = NULL; }

	s_bSharedAllocated = false;
}

void CGUITextureD3D::Allocate()
{
	m_pd3dDevice = g_graphicsContext.Get3DDevice();

	if (!m_pd3dDevice)
		return;

	// Ensure shared resources are created (shaders, vertex declaration)
	AllocateShared(m_pd3dDevice);
}

void CGUITextureD3D::Free()
{
	// No per-instance GPU resources to release
}

void CGUITextureD3D::Begin()
{
	LPDIRECT3DDEVICE9 p3DDevice = g_graphicsContext.Get3DDevice();

	// Set texture
	p3DDevice->SetTexture( 0, m_texture.m_textures[m_currentFrame] );

	if (m_diffuse.size())
	{
		p3DDevice->SetTexture( 1, m_diffuse.m_textures[0] );
		p3DDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		p3DDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	}

	// Render states and stage 0 sampler states are set once per frame
	// in CGraphicContext::ApplyStateBlock()

	// Set shared vertex declaration and shaders
	p3DDevice->SetVertexDeclaration(s_pVertexDecl);
	p3DDevice->SetVertexShader(s_pVertexShader);
	p3DDevice->SetPixelShader(m_diffuse.size() ? s_pPixelShaderDiffuse : s_pPixelShader);
}

void CGUITextureD3D::End()
{
	LPDIRECT3DDEVICE9 p3DDevice = g_graphicsContext.Get3DDevice();

	// Unset the texture or the texture caching crashes because the runtime still has a reference
	p3DDevice->SetTexture( 0, NULL );

	if (m_diffuse.size())
		p3DDevice->SetTexture( 1, NULL );
}

void CGUITextureD3D::Draw(float *x, float *y, float *z, const CRect &texture, const CRect &diffuse, color_t color, int orientation)
{
	// D3D aligns to half pixel boundaries
	for (int i = 0; i < 4; i++)
	{
		x[i] -= 0.5f;
		y[i] -= 0.5f;
	};

	// Build texture coordinates based on orientation
	float tu[4], tv[4], tu2[4], tv2[4];

	tu[0] = texture.x1;   tv[0] = texture.y1;
	tu2[0] = diffuse.x1;  tv2[0] = diffuse.y1;

	if (orientation & 4)
	{
		tu[1] = texture.x1; tv[1] = texture.y2;
	}
	else
	{
		tu[1] = texture.x2; tv[1] = texture.y1;
	}
	if (m_info.orientation & 4)
	{
		tu2[1] = diffuse.x1; tv2[1] = diffuse.y2;
	}
	else
	{
		tu2[1] = diffuse.x2; tv2[1] = diffuse.y1;
	}

	tu[2] = texture.x2;   tv[2] = texture.y2;
	tu2[2] = diffuse.x2;  tv2[2] = diffuse.y2;

	if (orientation & 4)
	{
		tu[3] = texture.x2; tv[3] = texture.y1;
	}
	else
	{
		tu[3] = texture.x1; tv[3] = texture.y2;
	}
	if (m_info.orientation & 4)
	{
		tu2[3] = diffuse.x2; tv2[3] = diffuse.y1;
	}
	else
	{
		tu2[3] = diffuse.x1; tv2[3] = diffuse.y2;
	}

	// Build vertices directly on stack (triangle strip order: 1,0,2,3)
	D3DCUSTOMVERTEX Vertices[4] =
	{
		{x[1], y[1], z[1], tu[1], tv[1], tu2[1], tv2[1]},
		{x[0], y[0], z[0], tu[0], tv[0], tu2[0], tv2[0]},
		{x[2], y[2], z[2], tu[2], tv[2], tu2[2], tv2[2]},
		{x[3], y[3], z[3], tu[3], tv[3], tu2[3], tv2[3]},
	};

	// Set per-draw constants
	m_pd3dDevice->SetVertexShaderConstantF(0, ( FLOAT* )&g_graphicsContext.GetFinalMatrix(), 4);

#pragma warning(push)
#pragma warning (disable:4244)
	float fInputAlpha[] = { (color >> 24 & 0xFF) / 255.0 };
	m_pd3dDevice->SetPixelShaderConstantF(10, fInputAlpha, 4);
#pragma warning(pop)

	// Draw directly from user pointer - no VB lock/unlock overhead
	m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, Vertices, sizeof(D3DCUSTOMVERTEX));
}

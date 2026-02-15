#include "legacyshaders.h"

//-------------------------------------------------------------------------------------
// Vertex shader
// We use the register semantic here to directly define the input register
// matWVP.  Conversely, we could let the HLSL compiler decide and check the
// constant table.
//-------------------------------------------------------------------------------------
#if 1
const char* g_strVertexShaderProgram_Legacy1 = 
" float4x4 matWVP : register(c0);              "  
"                                              "  
" struct VS_IN                                 "  
" {                                            " 
"     float4 ObjPos   : POSITION;              "  // Object space position 
"     float4 Color    : COLOR0;                "  // Vertex color
" };                                           " 
"                                              " 
" struct VS_OUT                                " 
" {                                            " 
"     float4 ProjPos  : POSITION;              "  // Projected position
"     float4 Color    : COLOR0;                "  // Pass-through color
" };                                           "  
"                                              "  
" VS_OUT main( VS_IN In )                      "  
" {                                            "  
"     VS_OUT Out;                              "  
"     Out.ProjPos = mul( matWVP, In.ObjPos );  "  
"     Out.Color = In.Color;                    "  // Pass through color
"     return Out;                              "  
" }";
#endif


typedef struct _WFVERTEX 
{
    float x, y, z;
    DWORD Diffuse;   // diffuse color. also acts as filler; aligns struct to 16 bytes (good for random access/indexed prims)
} WFVERTEX, *LPWFVERTEX; 

#define WFVERTEX_FORMAT     (D3DFVF_XYZ | D3DFVF_DIFFUSE )

// First Legacy Pixel Shader - #define WFVERTEX_FORMAT     (D3DFVF_XYZ | D3DFVF_DIFFUSE )
D3DVertexShader*       g_pVertexShader_Legacy1;

//=====================================================================================================

#if 1
const char* g_strVertexShaderProgram_Legacy2 = 
" float4x4 matWVP : register(c0);               \n"  
"                                               \n"  
" struct VS_IN                                  \n"  
" {                                             \n"  
"     float3 ObjPos   : POSITION;               \n"  // x, y, z
"     float4 Diffuse  : COLOR0;                 \n"  // Diffuse color (DWORD packed as ARGB)
"     float4 TexData  : TEXCOORD0;              \n"  // tu, tv, tu_orig, tv_orig (Packed Float4)
"     float2 RadAng   : TEXCOORD1;              \n"  // rad, ang (Moved to 1)
" };                                            \n"  
"                                               \n"  
" struct VS_OUT                                 \n"  
" {                                             \n"  
"     float4 ProjPos  : POSITION;               \n"  // Transformed position
"     float4 Diffuse  : COLOR0;                 \n"  // Pass-through color
"     float4 TexCoord : TEXCOORD0;              \n"  // uv, uv_orig packed
"     float2 RadAng   : TEXCOORD1;              \n"  // rad, ang
" };                                            \n"  
"                                               \n"  
" VS_OUT main(VS_IN In)                         \n"  
" {                                             \n"  
"     VS_OUT Out;                               \n"  
"                                               \n"  
"     // Transform position                     \n"  
"     Out.ProjPos = mul(matWVP, float4(In.ObjPos, 1.0)); \n"  
"                                               \n"  
"     // Pass-through parameters                \n"  
"     Out.Diffuse  = In.Diffuse;                \n"  
"     Out.TexCoord = In.TexData;                \n"  
"     Out.RadAng   = In.RadAng;                 \n"  
"                                               \n"  
"     return Out;                               \n"  
" }                                             \n";  
#endif

typedef struct _MYVERTEX 
{
    float x, y, z;     // screen position + Z-buffer depth    
    DWORD Diffuse;     // diffuse color    
    float tu, tv;           // DYNAMIC
     float tu_orig, tv_orig; // STATIC
    float rad, ang;         // STATIC
} MYVERTEX, *LPMYVERTEX; 

#define MYVERTEX_FORMAT     (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX3 | D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEXCOORDSIZE2(1) | D3DFVF_TEXCOORDSIZE2(2))  

D3DVertexShader*       g_pVertexShader_Legacy2;

//=====================================================================================================

// Vertex Shader (vs_2_0)
const char* g_strVertexShaderProgram_Legacy3 = 
" float4x4 matWVP : register(c0);              \n"
"                                              \n"
" struct VS_IN                                 \n"
" {                                            \n"
"     float3 ObjPos   : POSITION;              \n"  // x,y,z in object space
"     float4 Color    : COLOR0;                \n"  // diffuse tint
"     float2 TexCoord : TEXCOORD0;             \n"  // u,v
" };                                           \n"
"                                              \n"
" struct VS_OUT                                \n"
" {                                            \n"
"     float4 ProjPos  : POSITION;              \n"  // transformed pos
"     float4 Color    : COLOR0;                \n"
"     float2 TexCoord : TEXCOORD0;             \n"
" };                                           \n"
"                                              \n"
" VS_OUT main( VS_IN In )                      \n"
" {                                            \n"
"     VS_OUT Out;                              \n"
"     Out.ProjPos  = mul(matWVP, float4(In.ObjPos, 1));  \n"
"     Out.Color    = In.Color;                 \n"
"     Out.TexCoord = In.TexCoord;              \n"  // Pass through texture coordinates
"     return Out;                              \n"
" }";


// note: layout must match the vertex declaration in plugin.cpp!

typedef struct _SPRITEVERTEX 
{
    float x, y;      // screen position    
    float z;         // Z-buffer depth    
    DWORD Diffuse;   // diffuse color. also acts as filler; aligns struct to 16 bytes (good for random access/indexed prims)
    float tu, tv;    // texture coordinates for texture #0
} SPRITEVERTEX, *LPSPRITEVERTEX; 

#define SPRITEVERTEX_FORMAT (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0) )

D3DVertexShader*       g_pVertexShader_Legacy3;

//-------------------------------------------------------------------------------------
// Pixel shader
//-------------------------------------------------------------------------------------

#if 1
const char* g_strPixelShaderProgram_Legacy1 = 
" struct PS_IN                                 "
" {                                            "
"     float4 Color    : COLOR0;                "  // Matches vertex shader output
" };                                           "  
"                                              "  
" float4 main( PS_IN In ) : COLOR              "  
" {                                            "  
"     return In.Color;                         "  // Output Vertex Color
" }                                            ";
#endif

//=====================================================================================================

#if 1
const char* g_strPixelShaderProgram_Legacy2 = 
" sampler2D Tex0 : register(s0);               \n"  // Declare sampler for stage 0
" struct PS_IN                                 \n"
" {                                            \n"
"     float4 Color    : COLOR0;                \n"
"     float2 TexCoord : TEXCOORD0;             \n"  // Uses TEXCOORD0 (tu/tv)
" };                                           \n"
"                                              \n"
" float4 main(PS_IN In) : COLOR0               \n"
" {                                            \n"
"     float4 tex = tex2D(Tex0, In.TexCoord);   \n"
"     return float4(tex.rgb * In.Color.rgb, In.Color.a); \n"  // Alpha from vertex only (matches original FFP: ALPHAOP=SELECTARG1, ALPHAARG1=DTA_DIFFUSE)
" }                                            \n";
#endif

//=====================================================================================================

#if 1
const char* g_strPixelShaderProgram_Legacy3 =
" sampler2D Tex0 : register(s0);               \n"
"                                              \n"
" struct PS_IN                                 \n"
" {                                            \n"
"     float4 Color    : COLOR0;                \n"
"     float2 TexCoord : TEXCOORD0;             \n"
" };                                           \n"
"                                              \n"
" float4 main( PS_IN In ) : COLOR0             \n"
" {                                            \n"
"     float4 texcol = tex2D(Tex0, In.TexCoord);\n"
"                                              \n"
"     // Match original FFP: color = tex*diffuse, alpha = diffuse only \n"
"     return float4(texcol.rgb * In.Color.rgb, In.Color.a); \n"
" }";
#endif

//=====================================================================================================

D3DPixelShader*        g_pPixelShader_Legacy1;
D3DPixelShader*        g_pPixelShader_Legacy2;
D3DPixelShader*        g_pPixelShader_Legacy3;

void DoPixelShader_Legacy1(D3DDevice* pD3dDevice)
{
    // Compile vertex shader.
    ID3DXBuffer* pVertexShaderCode;
    ID3DXBuffer* pVertexErrorMsg;

    HRESULT hr = D3DXCompileShader( g_strVertexShaderProgram_Legacy1, 
                                    (UINT)strlen( g_strVertexShaderProgram_Legacy1 ),
                                    NULL, 
                                    NULL, 
                                    "main", 
                                    "vs_2_0", 
                                    0, 
                                    &pVertexShaderCode, 
                                    &pVertexErrorMsg, 
                                    NULL );
    if( FAILED(hr) )
    {
        if( pVertexErrorMsg )
            OutputDebugString( (char*)pVertexErrorMsg->GetBufferPointer() );
        return;
    }    

    // Create vertex shader.
    pD3dDevice->CreateVertexShader( (DWORD*)pVertexShaderCode->GetBufferPointer(), 
                                      &g_pVertexShader_Legacy1 );

    // Compile pixel shader.
    ID3DXBuffer* pPixelShaderCode;
    ID3DXBuffer* pPixelErrorMsg;
    hr = D3DXCompileShader( g_strPixelShaderProgram_Legacy1, 
                            (UINT)strlen( g_strPixelShaderProgram_Legacy1 ),
                            NULL, 
                            NULL, 
                            "main", 
                            "ps_2_0", 
                            0, 
                            &pPixelShaderCode, 
                            &pPixelErrorMsg,
                            NULL );
    if( FAILED(hr) )
    {
        if( pPixelErrorMsg )
            OutputDebugString( (char*)pPixelErrorMsg->GetBufferPointer() );
        return;
    }

    // Create pixel shader.
    pD3dDevice->CreatePixelShader( (DWORD*)pPixelShaderCode->GetBufferPointer(), 
                                     &g_pPixelShader_Legacy1 );
}

void DoPixelShader_Legacy2(D3DDevice* pD3dDevice)
{
    // Compile vertex shader.
    ID3DXBuffer* pVertexShaderCode;
    ID3DXBuffer* pVertexErrorMsg;

    HRESULT hr = D3DXCompileShader( g_strVertexShaderProgram_Legacy2, 
                                    (UINT)strlen( g_strVertexShaderProgram_Legacy2 ),
                                    NULL, 
                                    NULL, 
                                    "main", 
                                    "vs_2_0", 
                                    0, 
                                    &pVertexShaderCode, 
                                    &pVertexErrorMsg, 
                                    NULL );
    if( FAILED(hr) )
    {
        if( pVertexErrorMsg )
            OutputDebugString( (char*)pVertexErrorMsg->GetBufferPointer() );
        return;
    }    

    // Create vertex shader.
    pD3dDevice->CreateVertexShader( (DWORD*)pVertexShaderCode->GetBufferPointer(), 
                                      &g_pVertexShader_Legacy2 );

    // Compile pixel shader.
    ID3DXBuffer* pPixelShaderCode;
    ID3DXBuffer* pPixelErrorMsg;
    hr = D3DXCompileShader( g_strPixelShaderProgram_Legacy2, 
                            (UINT)strlen( g_strPixelShaderProgram_Legacy2 ),
                            NULL, 
                            NULL, 
                            "main", 
                            "ps_2_0", 
                            0, 
                            &pPixelShaderCode, 
                            &pPixelErrorMsg,
                            NULL );
    if( FAILED(hr) )
    {
        if( pPixelErrorMsg )
            OutputDebugString( (char*)pPixelErrorMsg->GetBufferPointer() );
        return;
    }

    // Create pixel shader.
    pD3dDevice->CreatePixelShader( (DWORD*)pPixelShaderCode->GetBufferPointer(), 
                                     &g_pPixelShader_Legacy2 );
    
}

void DoPixelShader_Legacy3(D3DDevice* pD3dDevice)
{
    // Compile vertex shader.
    ID3DXBuffer* pVertexShaderCode;
    ID3DXBuffer* pVertexErrorMsg;

    HRESULT hr = D3DXCompileShader( g_strVertexShaderProgram_Legacy3, 
                                    (UINT)strlen( g_strVertexShaderProgram_Legacy3 ),
                                    NULL, 
                                    NULL, 
                                    "main", 
                                    "vs_2_0", 
                                    0, 
                                    &pVertexShaderCode, 
                                    &pVertexErrorMsg, 
                                    NULL );
    if( FAILED(hr) )
    {
        if( pVertexErrorMsg )
            OutputDebugString( (char*)pVertexErrorMsg->GetBufferPointer() );
        return;
    }    

    // Create vertex shader.
    pD3dDevice->CreateVertexShader( (DWORD*)pVertexShaderCode->GetBufferPointer(), 
                                      &g_pVertexShader_Legacy3 );

    // Compile pixel shader.
    ID3DXBuffer* pPixelShaderCode;
    ID3DXBuffer* pPixelErrorMsg;
    hr = D3DXCompileShader( g_strPixelShaderProgram_Legacy3, 
                            (UINT)strlen( g_strPixelShaderProgram_Legacy3 ),
                            NULL, 
                            NULL, 
                            "main", 
                            "ps_2_0", 
                            0, 
                            &pPixelShaderCode, 
                            &pPixelErrorMsg,
                            NULL );
    if( FAILED(hr) )
    {
        if( pPixelErrorMsg )
            OutputDebugString( (char*)pPixelErrorMsg->GetBufferPointer() );
        return;
    }

    // Create pixel shader.
    pD3dDevice->CreatePixelShader( (DWORD*)pPixelShaderCode->GetBufferPointer(), 
                                     &g_pPixelShader_Legacy3 );
    
}

IDirect3DTexture9* g_pTextureLegacy1 = nullptr;
IDirect3DTexture9* g_pTextureLegacy2 = nullptr;
IDirect3DTexture9* g_pTextureLegacy3 = nullptr;

void CLegacyShaders::CreateDummyTextures(IDirect3DDevice9* pDevice)
{
    // Create 2x2 pink/black checkerboard for Legacy1 & Legacy3
    if (FAILED(D3DXCreateTexture(pDevice, 2, 2, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &g_pTextureLegacy1)))
        return;

    // Create 2x8 vertical stripes for Legacy2
    if (FAILED(D3DXCreateTexture(pDevice, 2, 8, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &g_pTextureLegacy2)))
        return;

    if (FAILED(D3DXCreateTexture(pDevice, 2, 2, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &g_pTextureLegacy3)))
        return;

    // Fill Legacy1 (2x2 checkerboard)
    D3DLOCKED_RECT lockedRect;
    if (SUCCEEDED(g_pTextureLegacy1->LockRect(0, &lockedRect, NULL, 0))) 
    {
        DWORD* pData = (DWORD*)lockedRect.pBits;
        const DWORD pink = 0xFFFF69B4;
        const DWORD black = 0xFF000000;
        
        // Row 0
        pData[0] = pink;    // (0,0)
        pData[1] = black;   // (1,0)
        
        // Row 1 (using pitch for row alignment)
        DWORD* pRow1 = (DWORD*)((BYTE*)pData + lockedRect.Pitch);
        pRow1[0] = black;   // (0,1)
        pRow1[1] = pink;    // (1,1)
        
        g_pTextureLegacy1->UnlockRect(0);
    }

    // Fill Legacy2 (2x8 vertical stripes)
    if (SUCCEEDED(g_pTextureLegacy2->LockRect(0, &lockedRect, NULL, 0))) 
    {
        DWORD* pData = (DWORD*)lockedRect.pBits;
        const DWORD pink = 0xFFFF69B4;
        const DWORD black = 0xFF000000;

        for (UINT y = 0; y < 8; y++) 
        {
            DWORD* pRow = (DWORD*)((BYTE*)pData + y * lockedRect.Pitch);
            pRow[0] = pink;   // Left column
            pRow[1] = black;  // Right column
        }
        g_pTextureLegacy2->UnlockRect(0);
    }

    // Fill Legacy3 (2x2 checkerboard)
    if (SUCCEEDED(g_pTextureLegacy3->LockRect(0, &lockedRect, NULL, 0))) 
    {
        DWORD* pData = (DWORD*)lockedRect.pBits;
        const DWORD pink = 0xFFFF69B4;
        const DWORD black = 0xFF000000;
        
        // Row 0
        pData[0] = pink;    // (0,0)
        pData[1] = black;   // (1,0)
        
        // Row 1
        DWORD* pRow1 = (DWORD*)((BYTE*)pData + lockedRect.Pitch);
        pRow1[0] = black;   // (0,1)
        pRow1[1] = pink;    // (1,1)
        
        g_pTextureLegacy3->UnlockRect(0);
    }

    return;
}

void CLegacyShaders::CreateShaders(D3DDevice* pD3dDevice)
{
	CreateDummyTextures(pD3dDevice);

	DoPixelShader_Legacy1(pD3dDevice);

	DoPixelShader_Legacy2(pD3dDevice);

	DoPixelShader_Legacy3(pD3dDevice);

	return;
}

void CLegacyShaders::DrawLegacyPrim1(D3DDevice* pD3dDevice)
{
    // Legacy1: Uses color's XY as TexCoord
    pD3dDevice->SetTexture(0, g_pTextureLegacy1);
    pD3dDevice->SetVertexShader(g_pVertexShader_Legacy1);
    pD3dDevice->SetPixelShader(g_pPixelShader_Legacy1);
    pD3dDevice->SetFVF(WFVERTEX_FORMAT);

    WFVERTEX verts1[3] = {
        // x, y, z, Diffuse (ARGB)
        {-0.5f, -0.5f, 0.5f, D3DCOLOR_ARGB(255, 0, 0, 0)},    // TexCoord (0,0)
        { 0.5f, -0.5f, 0.5f, D3DCOLOR_ARGB(255, 255, 0, 0)},  // TexCoord (1,0)
        { 0.0f, 0.5f, 0.5f, D3DCOLOR_ARGB(255, 0, 255, 0)},   // TexCoord (0,1)
    };

    pD3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, verts1, sizeof(WFVERTEX));
}

void CLegacyShaders::DrawLegacyPrim2(D3DDevice* pD3dDevice)
{
    // Legacy2: Uses explicit TexCoords
    pD3dDevice->SetTexture(0, g_pTextureLegacy2);
    pD3dDevice->SetVertexShader(g_pVertexShader_Legacy2);
    pD3dDevice->SetPixelShader(g_pPixelShader_Legacy2);
    pD3dDevice->SetFVF(MYVERTEX_FORMAT);

    MYVERTEX verts2[3] = {
        // x, y, z, Diffuse, tu, tv, tu_orig, tv_orig, rad, ang
        {-0.5f, -0.5f, 0.5f, D3DCOLOR_XRGB(255,255,255), 0.0f,0.0f, 0.0f,0.0f, 1.0f,0.0f},
        { 0.5f, -0.5f, 0.5f, D3DCOLOR_XRGB(255,255,255), 1.0f,0.0f, 1.0f,0.0f, 1.0f,0.0f},
        { 0.0f, 0.5f, 0.5f, D3DCOLOR_XRGB(255,255,255), 0.5f,1.0f, 0.5f,1.0f, 1.0f,0.0f},
    };
    pD3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, verts2, sizeof(MYVERTEX));
}

void CLegacyShaders::DrawLegacyPrim3(D3DDevice* pD3dDevice)
{
    // Legacy3: Simple texture + color
    pD3dDevice->SetTexture(0, g_pTextureLegacy3);
    pD3dDevice->SetVertexShader(g_pVertexShader_Legacy3);
    pD3dDevice->SetPixelShader(g_pPixelShader_Legacy3);
    pD3dDevice->SetFVF(SPRITEVERTEX_FORMAT);

    SPRITEVERTEX verts3[3] = {
        // x, y, z, Diffuse, tu, tv
        {-0.5f, -0.5f, 0.5f, D3DCOLOR_XRGB(255,255,255), 0.0f, 0.0f},
        { 0.5f, -0.5f, 0.5f, D3DCOLOR_XRGB(255,255,255), 1.0f, 0.0f},
        { 0.0f, 0.5f, 0.5f, D3DCOLOR_XRGB(255,255,255), 0.5f, 1.0f},
    };
    pD3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, verts3, sizeof(SPRITEVERTEX));
}

void CLegacyShaders::UnbindShaders(D3DDevice* pD3dDevice)
{
	if (pD3dDevice)
	{
		pD3dDevice->SetVertexShader(NULL);
		pD3dDevice->SetPixelShader(NULL);
		pD3dDevice->SetVertexDeclaration(NULL);
		pD3dDevice->SetFVF(0);

		// Clear all texture stages so the device doesn't hold
		// dangling pointers to textures we're about to release
		for (int i = 0; i < 16; i++)
			pD3dDevice->SetTexture(i, NULL);
	}
}

void CLegacyShaders::DeleteShaders()
{
    // Release dummy textures
    if (g_pTextureLegacy1)
    {
        g_pTextureLegacy1->Release();
        g_pTextureLegacy1 = NULL;
    }
    if (g_pTextureLegacy2)
    {
        g_pTextureLegacy2->Release();
        g_pTextureLegacy2 = NULL;
    }
    if (g_pTextureLegacy3)
    {
        g_pTextureLegacy3->Release();
        g_pTextureLegacy3 = NULL;
    }

    // Release vertex shaders
    if (g_pVertexShader_Legacy1)
    {
        g_pVertexShader_Legacy1->Release();
        g_pVertexShader_Legacy1 = NULL;
    }
    if (g_pVertexShader_Legacy2)
    {
        g_pVertexShader_Legacy2->Release();
        g_pVertexShader_Legacy2 = NULL;
    }
    if (g_pVertexShader_Legacy3)
    {
        g_pVertexShader_Legacy3->Release();
        g_pVertexShader_Legacy3 = NULL;
    }

    // Release pixel shaders
    if (g_pPixelShader_Legacy1)
    {
        g_pPixelShader_Legacy1->Release();
        g_pPixelShader_Legacy1 = NULL;
    }
    if (g_pPixelShader_Legacy2)
    {
        g_pPixelShader_Legacy2->Release();
        g_pPixelShader_Legacy2 = NULL;
    }
    if (g_pPixelShader_Legacy3)
    {
        g_pPixelShader_Legacy3->Release();
        g_pPixelShader_Legacy3 = NULL;
    }
}
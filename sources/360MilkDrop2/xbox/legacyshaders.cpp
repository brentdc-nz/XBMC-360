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
"     float4 Color    : COLOR;                 "  // Vertex color (used for TexCoord)
" };                                           " 
"                                              " 
" struct VS_OUT                                " 
" {                                            " 
"     float4 ProjPos  : POSITION;              "  // Projected position
"     float2 TexCoord : TEXCOORD0;             "  // Output as TEXCOORD0 (float2)
" };                                           "  
"                                              "  
" VS_OUT main( VS_IN In )                      "  
" {                                            "  
"     VS_OUT Out;                              "  
"     Out.ProjPos = mul( matWVP, In.ObjPos );  "  
"     Out.TexCoord = In.Color.xy;              "  // Use color's XY as TexCoord
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
"     float2 TexCoord : TEXCOORD0;              \n"  // tu, tv (dynamic)
"     float2 TexOrig  : TEXCOORD1;              \n"  // tu_orig, tv_orig (static)
"     float2 RadAng   : TEXCOORD2;              \n"  // rad, ang (static)
" };                                            \n"  
"                                               \n"  
" struct VS_OUT                                 \n"  
" {                                             \n"  
"     float4 ProjPos  : POSITION;               \n"  // Transformed position
"     float4 Diffuse  : COLOR0;                 \n"  // Pass-through color
"     float2 TexCoord : TEXCOORD0;              \n"  // Dynamic UVs
"     float2 TexOrig  : TEXCOORD1;              \n"  // Static UVs
"     float2 RadAng   : TEXCOORD2;              \n"  // rad, ang
"     float3 WorldPos : TEXCOORD3;              \n"  // Original x, y, z (optional)
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
"     Out.TexCoord = In.TexCoord;               \n"  
"     Out.TexOrig  = In.TexOrig;                \n"  
"     Out.RadAng   = In.RadAng;                 \n"  
"     Out.WorldPos = In.ObjPos;                 \n"  
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
"     Out.TexCoord = In.Color.yz;              "  // Use color's YZ (R and G) as TexCoord
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
"     float2 TexCoord : TEXCOORD0;             "  // Matches vertex shader
" };                                           "  
"                                              "  
" float4 main( PS_IN In ) : COLOR              "  
" {                                            "  
"     return float4(In.TexCoord, 0, 1);        "  // Output RGBA
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
"     return tex * In.Color;                   \n"
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
"     // Combine texture and vertex alpha      \n"
"     float4 outColor;                         \n"
"     outColor.rgb = texcol.rgb * In.Color.rgb;\n"
"     outColor.a   = texcol.a * In.Color.a;    \n"  // FIX: Multiply texture + vertex alpha
"                                              \n"
"     return outColor;                         \n"
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

void CLegacyShaders::CreateShaders(D3DDevice* pD3dDevice)
{
	DoPixelShader_Legacy1(pD3dDevice);
	DoPixelShader_Legacy2(pD3dDevice);
	DoPixelShader_Legacy3(pD3dDevice);

	return;
}

void CLegacyShaders::DeleteShaders()
{
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
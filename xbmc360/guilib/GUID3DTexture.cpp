#include "GUID3DTexture.h"
#include "GraphicContext.h"
#include "TextureManager.h"

namespace D3DTextureShaders
{
//-------------------------------------------------------------------------------------
// Vertex shader
// We use the register semantic here to directly define the input register
// matWVP.  Conversely, we could let the HLSL compiler decide and check the
// constant table.
//-------------------------------------------------------------------------------------
const char* g_strVertexShaderProgram =
    " float4x4 matWVP : register(c0);              "
    "                                              "
    " struct VS_IN                                 "
    " {                                            "
    "     float4 ObjPos   : POSITION;              "  // Object space position 
    "     float2 TexCoord : TEXCOORD;              "
    " };                                           "
    "                                              "
    " struct VS_OUT                                "
    " {                                            "
    "     float4 ProjPos  : POSITION;              "  // Projected space position 
    "     float2 TexCoord : TEXCOORD;              "
    " };                                           "
    "                                              "
    " VS_OUT main( VS_IN In )                      "
    " {                                            "
    "     VS_OUT Out;                              "
    "     Out.ProjPos = mul( matWVP, In.ObjPos );  "  // Transform vertex into
    "     Out.TexCoord = In.TexCoord;              "
    "     return Out;                              "
    " }                                            ";

//-------------------------------------------------------------------------------------
// Pixel shader
//-------------------------------------------------------------------------------------
const char* g_strPixelShaderProgram =
    " struct PS_IN                                 "
    " {                                            "
    "     float2 TexCoord : TEXCOORD;              "
    " };                                           "  // the vertex shader
    "                                              "
    " sampler detail;                              "
    "                                              "
    " float4 main( PS_IN In ) : COLOR              "
    " {                                            "
    "     return tex2D( detail, In.TexCoord );     "  // Output color
    " }                                            ";
}

CGUID3DTexture::CGUID3DTexture(float posX, float posY, float width, float height, const CTextureInfo& texture)
{
	m_strFilename = texture.filename;
	m_initialized = false;

	m_posX = posX;
	m_posY = posY;
	m_width = width;
	m_height = height;

	m_pd3dDevice = g_graphicsContext.Get3DDevice();
	m_pTexture = NULL;
	m_pVB = NULL;
	m_pVertexShader = NULL;
	m_pVertexDecl = NULL;
	m_pPixelShader = NULL;
}

CGUID3DTexture::~CGUID3DTexture()
{
}

void CGUID3DTexture::SetFileName(const CStdString &strFilename)
{
	m_strFilename = strFilename;
}

bool CGUID3DTexture::AllocResources()
{
	if(!m_pd3dDevice)
		return false;

	if(m_initialized)
		return false;

	if ( !g_graphicsContext.IsFullScreenVideo() )
		g_graphicsContext.Lock();

	// Compile vertex shader.
	ID3DXBuffer* pVertexShaderCode;
	ID3DXBuffer* pVertexErrorMsg;
   
	D3DXCompileShader( D3DTextureShaders::g_strVertexShaderProgram,
                                    ( UINT )strlen( D3DTextureShaders::g_strVertexShaderProgram ),
                                    NULL,
                                    NULL,
                                    "main",
                                    "vs_2_0",
                                    0,
                                    &pVertexShaderCode,
                                    &pVertexErrorMsg,
                                    NULL );

    // Create vertex shader.
    m_pd3dDevice->CreateVertexShader( ( DWORD* )pVertexShaderCode->GetBufferPointer(),
                                      &m_pVertexShader );

    // Compile pixel shader.
	ID3DXBuffer* pPixelShaderCode;
    ID3DXBuffer* pPixelErrorMsg;
	D3DXCompileShader( D3DTextureShaders::g_strPixelShaderProgram,
                            ( UINT )strlen( D3DTextureShaders::g_strPixelShaderProgram ),
                            NULL,
                            NULL,
                            "main",
                            "ps_2_0",
                            0,
                            &pPixelShaderCode,
                            &pPixelErrorMsg,
                            NULL );

    // Create pixel shader.
    m_pd3dDevice->CreatePixelShader( ( DWORD* )pPixelShaderCode->GetBufferPointer(),
                                     &m_pPixelShader );

    // Define the vertex elements and
    // Create a vertex declaration from the element descriptions.
    static const D3DVERTEXELEMENT9 VertexElements[3] =
    {
        { 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        D3DDECL_END()
    };
    m_pd3dDevice->CreateVertexDeclaration( VertexElements, &m_pVertexDecl );

    // Create the vertex buffer. Here we are allocating enough memory
    // (from the default pool) to hold all our 3 custom vertices. 
    m_pd3dDevice->CreateVertexBuffer( 4 * sizeof( COLORVERTEX ),
                                                  D3DUSAGE_WRITEONLY,
                                                  NULL,
                                                  D3DPOOL_MANAGED,
                                                  &m_pVB,
                                                  NULL );

    // Now we fill the vertex buffer. To do this, we need to Lock() the VB to
    // gain access to the vertices. This mechanism is required because the
    // vertex buffer may still be in use by the GPU. This can happen if the
    // CPU gets ahead of the GPU. The GPU could still be rendering the previous
    // frame.

	COLORVERTEX Vertices[] =
    {
        {(FLOAT)m_posX, (FLOAT)m_posY,  0.0f, 0, 0 ,},
        {(FLOAT)m_posX+m_width, (FLOAT)m_posY, 0.0f,  1, 0 ,},
        {(FLOAT)m_posX, (FLOAT)m_height+(FLOAT)m_posY, 0.0f,  0, 1 ,},
	    {(FLOAT)m_posX+(FLOAT)m_width, (FLOAT)m_posY+(FLOAT)m_height, 0.0f,  1, 1 ,},
    };

    COLORVERTEX* pVertices;
    m_pVB->Lock( 0, 0, ( void** )&pVertices, 0 );
    memcpy( pVertices, Vertices, 4 * sizeof( COLORVERTEX ) );
    m_pVB->Unlock();

	g_TextureManager.Load(m_strFilename, NULL);

	m_pTexture = g_TextureManager.GetTexture(m_strFilename);

	if(pVertexShaderCode)
		pVertexShaderCode->Release();

	if(pVertexErrorMsg)
		pVertexErrorMsg->Release();

	if(pPixelShaderCode)
		pPixelShaderCode->Release();

	if(pPixelErrorMsg)
		pPixelErrorMsg->Release();

	m_initialized = true;

	if ( !g_graphicsContext.IsFullScreenVideo() )
		g_graphicsContext.Unlock();

	return true;
}

bool CGUID3DTexture::FreeResources()
{
	if(!m_initialized)
		return false;

	m_initialized = false;

//	if ( !g_graphicsContext.IsFullScreenVideo() )
		g_graphicsContext.Lock();

	if(m_pVB)
	{
		m_pd3dDevice->SetStreamSource( NULL, NULL, NULL, NULL );
		m_pVB->Release();
		m_pVB = NULL;
	}

	if(m_pVertexShader)
	{
		m_pd3dDevice->SetVertexShader( NULL );
		m_pVertexShader->Release();
		m_pVertexShader = NULL;
	}

	if(m_pVertexDecl)
	{
		m_pd3dDevice->SetVertexDeclaration( NULL );
		m_pVertexDecl->Release();
		m_pVertexDecl = NULL;
	}

	if(m_pPixelShader)
	{
		m_pd3dDevice->SetPixelShader( NULL );
		m_pPixelShader->Release();
		m_pPixelShader = NULL;
	}

//	if ( !g_graphicsContext.IsFullScreenVideo() )	
		g_graphicsContext.Unlock();


	return true;
}

void CGUID3DTexture::Render()
{
	if( !m_initialized || !m_pd3dDevice || !m_pVertexShader || !m_pVB )
		return;

	if ( !g_graphicsContext.IsFullScreenVideo() )
		g_graphicsContext.Lock();

    // Initialize the world and view matrix
    D3DXMatrixIdentity( &m_matWorld );
    D3DXMatrixIdentity( &m_matView );

    // Initialize the projection matrix
	D3DXMatrixOrthoOffCenterLH(&m_matProj, 0, (float)g_graphicsContext.GetWidth(), (float)g_graphicsContext.GetHeight(), 0, 0.0f, 1.0f ); 

    // Build the world-view-projection matrix and pass it into the vertex shader
    D3DXMATRIX matWVP = m_matWorld * m_matView * m_matProj;
    m_pd3dDevice->SetVertexShaderConstantF( 0, ( FLOAT* )&matWVP, 4 );

    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );

    g_graphicsContext.Get3DDevice()->SetRenderState( D3DRS_FILLMODE,     D3DFILL_SOLID );
    g_graphicsContext.Get3DDevice()->SetRenderState( D3DRS_CULLMODE,     D3DCULL_CCW );
    g_graphicsContext.Get3DDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    g_graphicsContext.Get3DDevice()->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
    g_graphicsContext.Get3DDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	// We are passing the vertices down a "stream", so first we need
    // to specify the source of that stream, which is our vertex buffer. 
    // Then we need to let D3D know what vertex and pixel shaders to use. 
    m_pd3dDevice->SetVertexDeclaration( m_pVertexDecl );
	m_pd3dDevice->SetStreamSource( 0, m_pVB, 0, sizeof( COLORVERTEX ) );
    m_pd3dDevice->SetVertexShader( m_pVertexShader );
    m_pd3dDevice->SetPixelShader( m_pPixelShader );

	if(m_pTexture)
		m_pd3dDevice->SetTexture( 0, m_pTexture );

    // Draw the vertices in the vertex buffer
    m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

    m_pd3dDevice->SetTexture( 0,  NULL );

	if ( !g_graphicsContext.IsFullScreenVideo() )
		g_graphicsContext.Unlock();
}
#include "ScreensaverPlasma.h"
#include "..\GraphicContext.h"

namespace PlasmaScreensaverShaders
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

CScreensaverPlasma::CScreensaverPlasma()
{
	m_pd3dDevice = NULL;
	m_iScreenWidth = 0;
	m_iScreenHeight = 0;
	m_initialized = false;

	m_pVB = NULL;
	m_pVertexShader = NULL;
	m_pVertexDecl = NULL;
	m_pPixelShader = NULL;
	m_pTexture = NULL;
}

CScreensaverPlasma::~CScreensaverPlasma()
{
}

bool CScreensaverPlasma::Initialize()
{
	m_pd3dDevice = g_graphicsContext.Get3DDevice();

	if(!m_pd3dDevice)
		return false;

	if(m_initialized)
		return false;

	m_iScreenWidth = g_graphicsContext.GetWidth();
	m_iScreenHeight = g_graphicsContext.GetHeight();

	// Compile vertex shader.
	ID3DXBuffer* pVertexShaderCode;
	ID3DXBuffer* pVertexErrorMsg;
   
	D3DXCompileShader( PlasmaScreensaverShaders::g_strVertexShaderProgram,
                                    ( UINT )strlen( PlasmaScreensaverShaders::g_strVertexShaderProgram ),
                                    NULL,
                                    NULL,
                                    "main",
                                    "vs_2_0",
                                    0,
                                    &pVertexShaderCode,
                                    &pVertexErrorMsg,
                                    NULL );

    // Create vertex shader.
	g_graphicsContext.TLock();
    m_pd3dDevice->CreateVertexShader( ( DWORD* )pVertexShaderCode->GetBufferPointer(),
                                      &m_pVertexShader );
	g_graphicsContext.TUnlock();

    // Compile pixel shader.
	ID3DXBuffer* pPixelShaderCode;
    ID3DXBuffer* pPixelErrorMsg;

	D3DXCompileShader( PlasmaScreensaverShaders::g_strPixelShaderProgram,
                            ( UINT )strlen( PlasmaScreensaverShaders::g_strPixelShaderProgram ),
                            NULL,
                            NULL,
                            "main",
                            "ps_2_0",
                            0,
                            &pPixelShaderCode,
                            &pPixelErrorMsg,
                            NULL );

    // Create pixel shader.
	g_graphicsContext.TLock();
    m_pd3dDevice->CreatePixelShader( ( DWORD* )pPixelShaderCode->GetBufferPointer(),
                                     &m_pPixelShader );
	g_graphicsContext.TUnlock();

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
	if(pVertexShaderCode)
		pVertexShaderCode->Release();

	if(pVertexErrorMsg)
		pVertexErrorMsg->Release();

	if(pPixelShaderCode)
		pPixelShaderCode->Release();

	if(pPixelErrorMsg)
		pPixelErrorMsg->Release();

	m_initialized = true;

	int iPosX = 0;
	int iPosY = 0;
	int iWidth = m_iScreenWidth;
	int iHeight = m_iScreenHeight;

	COLORVERTEX Vertices[] =
    {
        {(FLOAT)iPosX, (FLOAT)iPosY,  0.0f, 0, 0 ,},
        {(FLOAT)iPosX+iWidth, (FLOAT)iPosY, 0.0f,  1, 0 ,},
        {(FLOAT)iPosX, (FLOAT)iHeight+(FLOAT)iPosY, 0.0f,  0, 1 ,},
	    {(FLOAT)iPosX+(FLOAT)iWidth, (FLOAT)iPosY+(FLOAT)iHeight, 0.0f,  1, 1 ,},
    };

    COLORVERTEX* pVertices;
    m_pVB->Lock( 0, 0, ( void** )&pVertices, 0 );
    memcpy( pVertices, Vertices, 4 * sizeof( COLORVERTEX ) );
    m_pVB->Unlock();

    // Initialize the world and view matrix
    D3DXMatrixIdentity( &m_matWorld );
    D3DXMatrixIdentity( &m_matView );

    // Initialize the projection matrix
	D3DXMatrixOrthoOffCenterLH(&m_matProj, 0, (float)m_iScreenWidth, (float)m_iScreenHeight, 0, 0.0f, 1.0f ); 

	if(!m_pTexture)
	{
		//Create Texture
		g_graphicsContext.TLock();
		m_pd3dDevice->CreateTexture(320,
			200,
			1,
			0,
			D3DFMT_LIN_A8R8G8B8,
			D3DPOOL_MANAGED,
			&m_pTexture,
			NULL);
		g_graphicsContext.TUnlock();
	}

	for(int x = 0; x < 256; x++)
	{
		m_r[x] = 255 - (unsigned char)ceil((sin(3.14 * 2 * x / 255) + 1) * 127);
		m_g[x] = (unsigned char)ceil((sin(3.14 * 2 * x / 127.0) + 1) * 64);
		m_b[x] = 255 - m_r[x];
	}

	return true;
}

void CScreensaverPlasma::Render()
{
	uint32_t last_time;
	double f = 0;
	float c1;
	float c2;
	float c3;

	int x, y;

	int res;

	last_time = GetTickCount();

	for(y = 0; y < 200; y++)
	{
		for(x = 0; x < 320; x++)
		{
			c1 = (float)sin(x / 50.0 + f + y / 200.0);
			c2 = (float)sqrt((sin(0.8 * f) * 160 - x + 160) * (sin(0.8 * f) * 160 - x + 160) + (cos(1.2 * f) * 100 - y + 100) * (cos(1.2 * f) * 100 - y + 100));
			c2 = (float)sin(c2 / 50.0);
			c3 = (c1 + c2) / 2;

			res = (int)ceil((c3 + 1) * 127);

			D3DLOCKED_RECT lockrect;

			D3DCOLOR rgb;

			rgb = D3DCOLOR_ARGB(255, m_r[res], m_g[res], m_b[res]);

			m_pTexture->LockRect( 0, &lockrect, NULL, 0);

			((D3DCOLOR *)lockrect.pBits)[x+(lockrect.Pitch>>2)*y]=rgb;

			m_pTexture->UnlockRect(0);
		}
	}

	UpdateTexture();

	f += 0.1;
	if(GetTickCount() - last_time < 40)
	{
//		Sleep(40 - (GetTickCount() - last_time)); //FIXME - Why so slow?
	}

	last_time = GetTickCount();
}

bool CScreensaverPlasma::Close()
{
	if(!m_initialized)
		return false;

	if(m_pTexture)
	{
		g_graphicsContext.TLock();
		m_pTexture->Release();
		g_graphicsContext.TUnlock();
		m_pTexture = NULL;
	}

	if(m_pVB)
	{
		g_graphicsContext.TLock();
		m_pd3dDevice->SetStreamSource( NULL, NULL, NULL, NULL );
		g_graphicsContext.TUnlock();
		g_graphicsContext.TLock();
		m_pVB->Release();
		g_graphicsContext.TUnlock();
		m_pVB = NULL;
	}

	if(m_pVertexShader)
	{
		g_graphicsContext.TLock();
		m_pd3dDevice->SetVertexShader( NULL );
		g_graphicsContext.TUnlock();
		g_graphicsContext.TLock();
		m_pVertexShader->Release();
		g_graphicsContext.TUnlock();
		m_pVertexShader = NULL;
	}

	if(m_pVertexDecl)
	{
		g_graphicsContext.TLock();
		m_pd3dDevice->SetVertexDeclaration( NULL );
		g_graphicsContext.TUnlock();
		g_graphicsContext.TLock();
		m_pVertexDecl->Release();
		g_graphicsContext.TUnlock();
		m_pVertexDecl = NULL;
	}

	if(m_pPixelShader)
	{
		g_graphicsContext.TLock();
		m_pd3dDevice->SetPixelShader( NULL );
		g_graphicsContext.TUnlock();
		g_graphicsContext.TLock();
		m_pPixelShader->Release();
		g_graphicsContext.TUnlock();
		m_pPixelShader = NULL;
	}

	return true;
}

void CScreensaverPlasma::UpdateTexture()
{
	// Build the world-view-projection matrix and pass it into the vertex shader
	D3DXMATRIX matWVP = m_matWorld * m_matView * m_matProj;
	g_graphicsContext.TLock();
	m_pd3dDevice->SetVertexShaderConstantF( 0, ( FLOAT* )&matWVP, 4 );
	g_graphicsContext.TUnlock();

	g_graphicsContext.TLock();
	m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC );
	g_graphicsContext.TUnlock();
	g_graphicsContext.TLock();
	m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC );
	g_graphicsContext.TUnlock();
	g_graphicsContext.TLock();
	m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
	g_graphicsContext.TUnlock();

	// We are passing the vertices down a "stream", so first we need
    // to specify the source of that stream, which is our vertex buffer. 
    // Then we need to let D3D know what vertex and pixel shaders to use. 
	g_graphicsContext.TLock(); 
	m_pd3dDevice->SetVertexDeclaration( m_pVertexDecl );
	g_graphicsContext.TUnlock();
	g_graphicsContext.TLock(); 
	m_pd3dDevice->SetStreamSource( 0, m_pVB, 0, sizeof( COLORVERTEX ) );
	g_graphicsContext.TUnlock();
	g_graphicsContext.TLock();  
	m_pd3dDevice->SetVertexShader( m_pVertexShader );
	g_graphicsContext.TUnlock();
	g_graphicsContext.TLock();  
    m_pd3dDevice->SetPixelShader( m_pPixelShader );
	g_graphicsContext.TUnlock();

	g_graphicsContext.TLock(); 
	m_pd3dDevice->SetTexture( 0, m_pTexture );
	g_graphicsContext.TUnlock();

    // Draw the vertices in the vertex buffer
	g_graphicsContext.TLock(); 
	m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
	g_graphicsContext.TUnlock();
	g_graphicsContext.TLock();
	m_pd3dDevice->SetTexture( 0,  NULL );
	g_graphicsContext.TUnlock();

	g_graphicsContext.TLock();
	m_pd3dDevice->SetStreamSource( NULL, NULL, NULL, NULL );
	g_graphicsContext.TUnlock();
}
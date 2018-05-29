#include "RGBRenderer.h"
#include "..\..\guilib\GUIWindowManager.h"
#include "..\..\Application.h"
#include "..\..\guilib\GraphicContext.h"
#include "..\..\utils\Log.h"

namespace RGBRendererShaders
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

CRGBRenderer::CRGBRenderer(LPDIRECT3DDEVICE9 pDevice)
{
	m_pd3dDevice = pDevice;
	m_pTexture = NULL;
	m_pVB = NULL;
	m_pVertexShader = NULL;
	m_pVertexDecl = NULL;
	m_pPixelShader = NULL;

	m_iSourceWidth = 0;
	m_iSourceHeight = 0;

	m_iScreenWidth = 0;
	m_iScreenHeight = 0;

	m_iActiveWidth = 0;
	m_iActiveHeight = 0;
	m_iActivePosX = 0;
	m_iActivePosY = 0;

	m_initialized = false;

	m_bPrepared = false;
}

CRGBRenderer::~CRGBRenderer()
{
}

void CRGBRenderer::RenderUpdate(bool clear, DWORD flags, DWORD alpha)
{
	if(!m_pTexture)
		return;

	g_graphicsContext.Lock();

	ManageDisplay();

	if(clear)
		m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET, 0xff000000, 1.0f, 0L );

	g_graphicsContext.Unlock();

	Render();
}

bool CRGBRenderer::PreInit()
{
	if(!m_pd3dDevice)
		return false;

	if(m_initialized)
		return false;

	g_graphicsContext.Lock();

	m_iScreenWidth = g_graphicsContext.GetWidth();
	m_iScreenHeight = g_graphicsContext.GetHeight();

	// Compile vertex shader.
	ID3DXBuffer* pVertexShaderCode;
	ID3DXBuffer* pVertexErrorMsg;
   
	D3DXCompileShader( RGBRendererShaders::g_strVertexShaderProgram,
                                    ( UINT )strlen( RGBRendererShaders::g_strVertexShaderProgram ),
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

	D3DXCompileShader( RGBRendererShaders::g_strPixelShaderProgram,
                            ( UINT )strlen( RGBRendererShaders::g_strPixelShaderProgram ),
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
	if(pVertexShaderCode)
		pVertexShaderCode->Release();

	if(pVertexErrorMsg)
		pVertexErrorMsg->Release();

	if(pPixelShaderCode)
		pPixelShaderCode->Release();

	if(pPixelErrorMsg)
		pPixelErrorMsg->Release();

	m_initialized = true;

	g_graphicsContext.Unlock();

	return true;
}

void CRGBRenderer::ManageDisplay()
{
	int iPosX;
	int iPosY;
	int iWidth;
	int iHeight;

	if (g_windowManager.GetActiveWindow() == WINDOW_FULLSCREEN_VIDEO)
	{
		iPosX = 0;
		iPosY = 0;
		iWidth = m_iScreenWidth;
		iHeight = m_iScreenHeight;
	}
	else
	{
		const RECT& rv = g_graphicsContext.GetViewWindow();

		iPosX = rv.left;
		iPosY = rv.top;
		iWidth = rv.right - rv.left;
		iHeight = rv.bottom - rv.top;
	}

	if(m_iActiveWidth == iWidth && m_iActiveHeight == iHeight
		&& m_iActivePosX == iPosX && m_iActivePosY == iPosY)
		return;

	m_iActiveWidth = iWidth;
	m_iActiveHeight = iHeight;
	m_iActivePosX = iPosX;
	m_iActivePosY = iPosY;

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
}

bool CRGBRenderer::Configure(int iWidth, int iHeight)
{
	g_graphicsContext.Lock();

	m_iSourceWidth = iWidth;
	m_iSourceHeight = iHeight;

	if(!m_pTexture)
	{
		//Create Texture
		m_pd3dDevice->CreateTexture(m_iSourceWidth,
			m_iSourceHeight,
			1,
			0,
			D3DFMT_LIN_A8R8G8B8,
			D3DPOOL_MANAGED,
			&m_pTexture,
			NULL);
	}

	g_graphicsContext.Unlock();

	return true;
}

bool CRGBRenderer::GetImage(RGB32Image_t* image)
{
	g_graphicsContext.Lock();

	if (!image) return false;

	image->width = m_iSourceWidth;
	image->height = m_iSourceHeight;

	D3DLOCKED_RECT lockedRect;

	m_pTexture->LockRect(0, &lockedRect, NULL, NULL);

	image->plane = (BYTE*)lockedRect.pBits;
	image->stride = lockedRect.Pitch;

	return true;
}

void CRGBRenderer::ReleaseImage()
{
	m_pTexture->UnlockRect(0);

	g_graphicsContext.Unlock();
}

void CRGBRenderer::Render()
{
	g_graphicsContext.Lock();

	if( !m_pd3dDevice || !m_pVertexShader || !m_pVB )
	{
		CLog::Log(LOGERROR, "Direct3D rendering objects missing!");
		return;
	}

    // Build the world-view-projection matrix and pass it into the vertex shader
    D3DXMATRIX matWVP = m_matWorld * m_matView * m_matProj;
    m_pd3dDevice->SetVertexShaderConstantF( 0, ( FLOAT* )&matWVP, 4 );

    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );

	// We are passing the vertices down a "stream", so first we need
	// to specify the source of that stream, which is our vertex buffer. 
	// Then we need to let D3D know what vertex and pixel shaders to use. 
	m_pd3dDevice->SetVertexDeclaration( m_pVertexDecl );
	m_pd3dDevice->SetStreamSource( 0, m_pVB, 0, sizeof( COLORVERTEX ) );
	m_pd3dDevice->SetVertexShader( m_pVertexShader );
	m_pd3dDevice->SetPixelShader( m_pPixelShader );

	m_pd3dDevice->SetTexture( 0, m_pTexture );
	// Draw the vertices in the vertex buffer
	m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
	m_pd3dDevice->SetTexture( 0,  NULL );

	m_pd3dDevice->SetStreamSource( NULL, NULL, NULL, NULL );

	g_graphicsContext.Unlock();
}

void CRGBRenderer::PrepareDisplay()
{
	if (g_graphicsContext.IsFullScreenVideo() )
	{    
//		CSingleLock lock(g_graphicsContext);
		g_graphicsContext.Lock();
		
		ManageDisplay();

		m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET, 0xff000000, 1.0f, 0L );

		g_graphicsContext.Unlock();

		Render();

		g_graphicsContext.Lock();

		if (g_application.NeedRenderFullScreen())
		{ 
			// render our subtitles and osd
			g_application.RenderFullScreen();
		}
    
//		m_pD3DDevice->KickPushBuffer();

		g_graphicsContext.Unlock();
	}

	m_bPrepared = true;
}

void CRGBRenderer::FlipPage()
{
//	CSingleLock lock(g_graphicsContext);
	if( !m_bPrepared )
	{
		//This will prepare for rendering, ie swapping buffers and in fullscreen even rendering
		//it can have been done way earlier
		PrepareDisplay();
	}
	m_bPrepared=false;

	if ( g_graphicsContext.IsFullScreenVideo() && !g_application.m_pPlayer->IsPaused())
	{   
		g_graphicsContext.Lock();

		//Make sure the push buffer is done before waiting for vblank, otherwise we can get tearing
		while( m_pd3dDevice->IsBusy() ) Sleep(1);

		// Present the backbuffer contents to the display
		m_pd3dDevice->Present( NULL, NULL, NULL, NULL );

		g_graphicsContext.Unlock();
	}
}

void CRGBRenderer::UnInit()
{
	if(!m_initialized)
		return;

	g_graphicsContext.Lock();

	if(m_pTexture)
	{
		m_pTexture = NULL;
	}

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

	g_graphicsContext.Unlock();
}

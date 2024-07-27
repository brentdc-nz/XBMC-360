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
// matWVP. Conversely, we could let the HLSL compiler decide and check the
// constant table
//-------------------------------------------------------------------------------------
const char* g_strVertexShaderProgram =
	" float4x4 matWVP : register(c0);              "
	"                                              "
	" struct VS_IN                                 "
	" {                                            "
	"     float4 ObjPos   : POSITION;              "  // Object space position 
	"     float2 TexCoord : TEXCOORD0;             "
	" };                                           "
	"                                              "
	" struct VS_OUT                                "
	" {                                            "
	"     float4 ProjPos  : POSITION;              "  // Projected space position 
	"     float2 TexCoord : TEXCOORD0;             "
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
" sampler2D  YTexture : register( s0 );			"
" sampler2D  UTexture : register( s1 );			"
" sampler2D  VTexture : register( s2 );			"
" struct PS_IN                                 "
" {                                            "
"     float2 Uv : TEXCOORD0;                    "  // Interpolated color from                      
" };                                           "   // the vertex shader
"                                              "  
" float4 main( PS_IN In ) : COLOR              "  
" {                                            " 
"												"
"		float4 Y_4D = tex2D( YTexture, In.Uv );  "
"		float4 U_4D = tex2D( UTexture, In.Uv );  "
"		float4 V_4D = tex2D( VTexture, In.Uv );  "
"                                             "
"		float R = 1.164 * ( Y_4D.r - 0.0625 ) + 1.596 * ( V_4D.r - 0.5 ); "
"		float G = 1.164 * ( Y_4D.r - 0.0625 ) - 0.391 * ( U_4D.r - 0.5 ) - 0.813 * ( V_4D.r - 0.5 ); "
"		float B = 1.164 * ( Y_4D.r - 0.0625 ) + 2.018 * ( U_4D.r - 0.5 );          "                
"                            "                 
"		float4 ARGB;     "                        
"		ARGB.a = 1.0;     "                       
"		ARGB.r = R;        "                      
"		ARGB.g = G;         "                     
"		ARGB.b = B;          "                    
"                            "                 
"		return ARGB;  "
"					"		
" }                                            "; 
}

CRGBRenderer::CRGBRenderer(LPDIRECT3DDEVICE9 pDevice)
{
	m_pd3dDevice = pDevice;

	m_pFrameU = NULL;
	m_pFrameV = NULL;
	m_pFrameY = NULL;

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

	m_bInitialized = false;
	m_bConfigured = false;
	m_bPrepared = false;
}

CRGBRenderer::~CRGBRenderer()
{
}

void CRGBRenderer::Update(bool bPauseDrawing)
{
	if (!m_bConfigured) return;

	CSingleLock lock(g_graphicsContext);
	ManageDisplay();
}

void CRGBRenderer::RenderUpdate(bool clear, DWORD flags, DWORD alpha)
{
	if(!m_pFrameY || !m_pFrameU || !m_pFrameV)
		return;

	ManageDisplay();

	if(clear)
	{
		g_graphicsContext.TLock();
		m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET, 0xff000000, 1.0f, 0L );
		g_graphicsContext.TUnlock();
	}

	Render();
}

bool CRGBRenderer::PreInit()
{
	if(!m_pd3dDevice)
		return false;

	if(m_bInitialized)
		return false;

	m_bConfigured = false;

	m_iScreenWidth = g_graphicsContext.GetWidth();
	m_iScreenHeight = g_graphicsContext.GetHeight();

	// Compile vertex shader
	ID3DXBuffer* pVertexShaderCode;
	ID3DXBuffer* pVertexErrorMsg;
   
	D3DXCompileShader(RGBRendererShaders::g_strVertexShaderProgram,
                                    (UINT)strlen(RGBRendererShaders::g_strVertexShaderProgram),
                                    NULL,
                                    NULL,
                                    "main",
                                    "vs_2_0",
                                    0,
                                    &pVertexShaderCode,
                                    &pVertexErrorMsg,
                                    NULL);
	g_graphicsContext.TLock();

	// Create vertex shader
	m_pd3dDevice->CreateVertexShader((DWORD*)pVertexShaderCode->GetBufferPointer(), &m_pVertexShader);

	// Compile pixel shader
	ID3DXBuffer* pPixelShaderCode;
	ID3DXBuffer* pPixelErrorMsg;

	D3DXCompileShader(RGBRendererShaders::g_strPixelShaderProgram,
                            (UINT)strlen( RGBRendererShaders::g_strPixelShaderProgram),
                            NULL,
                            NULL,
                            "main",
                            "ps_2_0",
                            0,
                            &pPixelShaderCode,
                            &pPixelErrorMsg,
                            NULL);

    // Create pixel shader.
    m_pd3dDevice->CreatePixelShader((DWORD*)pPixelShaderCode->GetBufferPointer(), &m_pPixelShader);

    // Define the vertex elements and
    // create a vertex declaration from the element descriptions
    static const D3DVERTEXELEMENT9 VertexElements[3] =
    {
        { 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        D3DDECL_END()
    };

    m_pd3dDevice->CreateVertexDeclaration( VertexElements, &m_pVertexDecl );

    // Create the vertex buffer. Here we are allocating enough memory
    // (from the default pool) to hold all our 3 custom vertices. 
	m_pd3dDevice->CreateVertexBuffer(4 * sizeof(COLORVERTEX),
                                                  D3DUSAGE_WRITEONLY,
                                                  NULL,
                                                  D3DPOOL_MANAGED,
                                                  &m_pVB,
                                                  NULL);
	g_graphicsContext.TUnlock();

	if(pVertexShaderCode)
		pVertexShaderCode->Release();

	if(pVertexErrorMsg)
		pVertexErrorMsg->Release();

	if(pPixelShaderCode)
		pPixelShaderCode->Release();

	if(pPixelErrorMsg)
		pPixelErrorMsg->Release();

	m_bInitialized = true;

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
	m_pVB->Lock(0, 0, ( void** )&pVertices, 0);
	memcpy(pVertices, Vertices, 4 * sizeof(COLORVERTEX));
	m_pVB->Unlock();
}

bool CRGBRenderer::Configure(int iWidth, int iHeight)
{
	m_iSourceWidth = iWidth;
	m_iSourceHeight = iHeight;

	if(!m_pFrameY)
	{
		m_pd3dDevice->CreateTexture(m_iSourceWidth,
			m_iSourceHeight,
			1,
			0,
			D3DFMT_LIN_L8,
			D3DPOOL_MANAGED,
			&m_pFrameY,
			NULL);
	}

	if(!m_pFrameU)
	{
		m_pd3dDevice->CreateTexture(m_iSourceWidth>>1,
			m_iSourceHeight>>1,
			1,
			0,
			D3DFMT_LIN_L8,
			D3DPOOL_MANAGED,
			&m_pFrameU,
			NULL);
	}

	if(!m_pFrameV)
	{
		m_pd3dDevice->CreateTexture(m_iSourceWidth>>1,
			m_iSourceHeight>>1,
			1,
			0,
			D3DFMT_LIN_L8,
			D3DPOOL_MANAGED,
			&m_pFrameV,
			NULL);
	}

	m_bConfigured = true;

	return true;
}

bool CRGBRenderer::GetImage(YV12Image* image)
{
	if (!image) return false;

	image->width = m_iSourceWidth;
	image->height = m_iSourceHeight;

	D3DLOCKED_RECT lockRectY;
	D3DLOCKED_RECT lockRectU;
	D3DLOCKED_RECT lockRectV;

	g_graphicsContext.TLock();

	m_pFrameY->LockRect(0, &lockRectY, NULL, 0);
	m_pFrameU->LockRect(0, &lockRectU, NULL, 0);
	m_pFrameV->LockRect(0, &lockRectV, NULL, 0);

	g_graphicsContext.TUnlock();

	image->plane[0] = (uint8_t*)lockRectY.pBits;
	image->plane[1] = (uint8_t*)lockRectU.pBits;
	image->plane[2] = (uint8_t*)lockRectV.pBits;

	image->stride[0] = lockRectY.Pitch;
	image->stride[1] = lockRectU.Pitch;
	image->stride[2] = lockRectV.Pitch;

	return true;
}

void CRGBRenderer::ReleaseImage()
{
	m_pFrameU->UnlockRect(0);
	m_pFrameY->UnlockRect(0);
	m_pFrameV->UnlockRect(0);
}

void CRGBRenderer::Render()
{
	if( !m_pd3dDevice || !m_pVertexShader || !m_pVB )
	{
		CLog::Log(LOGERROR, "Direct3D rendering objects missing!");
		return;
	}

	g_graphicsContext.TLock();

	// Pass matrix into the vertex shader
	m_pd3dDevice->SetVertexShaderConstantF(0, (FLOAT*)&g_graphicsContext.GetFinalMatrix(), 4);

	// We are passing the vertices down a "stream", so first we need
	// to specify the source of that stream, which is our vertex buffer
	// Then we need to let D3D know what vertex and pixel shaders to use

	m_pd3dDevice->SetVertexDeclaration(m_pVertexDecl);

	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(COLORVERTEX));
	m_pd3dDevice->SetVertexShader(m_pVertexShader);
	m_pd3dDevice->SetPixelShader(m_pPixelShader);

	m_pd3dDevice->SetTexture(0, m_pFrameY);
	m_pd3dDevice->SetTexture(1, m_pFrameU);
	m_pd3dDevice->SetTexture(2, m_pFrameV);

	m_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	m_pd3dDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_pd3dDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_pd3dDevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	m_pd3dDevice->SetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_pd3dDevice->SetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_pd3dDevice->SetSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	// Draw the vertices in the vertex buffer
	m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

	m_pd3dDevice->SetTexture(0, NULL);
	m_pd3dDevice->SetTexture(1, NULL);
	m_pd3dDevice->SetTexture(2, NULL);

	m_pd3dDevice->SetStreamSource(NULL, NULL, NULL, NULL);

	g_graphicsContext.TUnlock();
}

void CRGBRenderer::PrepareDisplay()
{
	if(g_graphicsContext.IsFullScreenVideo())
	{    		
		ManageDisplay();

		g_graphicsContext.TLock();
		m_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET, 0xff000000, 1.0f, 0L);
		g_graphicsContext.TUnlock();

		Render();

		if(g_application.NeedRenderFullScreen())
		{ 
			// Render our subtitles and OSD
			g_application.RenderFullScreen();
		}
    
		if (!g_application.IsPaused())
		{
			g_application.RenderMemoryStatus();
		}

//		m_pd3dDevice->KickPushBuffer();
	}

	m_bPrepared = true;
}

void CRGBRenderer::FlipPage()
{
	if(!m_bPrepared)
	{
		// This will prepare for rendering, ie swapping buffers and in fullscreen even rendering
		// it can have been done way earlier
		PrepareDisplay();
	}

	m_bPrepared = false;

	if(g_graphicsContext.IsFullScreenVideo() && !g_application.m_pPlayer->IsPaused())
	{   
		// Make sure the push buffer is done before waiting for vblank, otherwise we can get tearing
		g_graphicsContext.TLock();
		while(m_pd3dDevice->IsBusy()) Sleep(1);
		g_graphicsContext.TUnlock();

		// Present the backbuffer contents to the display
		g_graphicsContext.TLock();
		m_pd3dDevice->Present(NULL, NULL, NULL, NULL);
		g_graphicsContext.TUnlock();
	}
}

void CRGBRenderer::UnInit()
{
	if(!m_bInitialized)
		return;

	if(m_pFrameY)
	{
		g_graphicsContext.TLock();
		m_pFrameY->Release();
		g_graphicsContext.TUnlock();
		m_pFrameY = NULL;
	}
	
	if(m_pFrameU)
	{
		g_graphicsContext.TLock();
		m_pFrameU->Release();
		g_graphicsContext.TUnlock();
		m_pFrameU = NULL;
	}

	if(m_pFrameV)
	{
		g_graphicsContext.TLock();
		m_pFrameV->Release();
		g_graphicsContext.TUnlock();
		m_pFrameV = NULL;
	}

	if(m_pVB)
	{
		g_graphicsContext.TLock();
		m_pd3dDevice->SetStreamSource(NULL, NULL, NULL, NULL);
		g_graphicsContext.TUnlock();
		g_graphicsContext.TLock();
		m_pVB->Release();
		g_graphicsContext.TUnlock();
		m_pVB = NULL;
	}

	if(m_pVertexShader)
	{
		g_graphicsContext.TLock();
		m_pd3dDevice->SetVertexShader(NULL);
		g_graphicsContext.TUnlock();
		g_graphicsContext.TLock();
		m_pVertexShader->Release();
		g_graphicsContext.TUnlock();
		m_pVertexShader = NULL;
	}

	if(m_pVertexDecl)
	{
		g_graphicsContext.TLock();
		m_pd3dDevice->SetVertexDeclaration(NULL);
		g_graphicsContext.TUnlock();
		g_graphicsContext.TLock();
		m_pVertexDecl->Release();
		g_graphicsContext.TUnlock();
		m_pVertexDecl = NULL;
	}

	if(m_pPixelShader)
	{
		g_graphicsContext.TLock();
		m_pd3dDevice->SetPixelShader(NULL);
		g_graphicsContext.TUnlock();
		g_graphicsContext.TLock();
		m_pPixelShader->Release();
		g_graphicsContext.TUnlock();
		m_pPixelShader = NULL;
	}

	m_bConfigured = false;
}

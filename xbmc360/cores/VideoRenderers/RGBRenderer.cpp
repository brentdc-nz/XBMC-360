#include "RGBRenderer.h"
#include "..\..\guilib\GUIWindowManager.h"
#include "..\..\Application.h"
#include "..\..\guilib\GraphicContext.h"
#include "..\..\utils\Log.h"
#include "..\..\Settings.h"

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

	m_fSourceFrameRatio = 1.0f;
	memset(&rd, 0, sizeof(rd));

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
	const RECT& rv = g_graphicsContext.GetViewWindow();
	float fScreenWidth = (float)rv.right - rv.left;
	float fScreenHeight = (float)rv.bottom - rv.top;
	float fOffsetX1 = (float)rv.left;
	float fOffsetY1 = (float)rv.top;

	CalcNormalDisplayRect(fOffsetX1, fOffsetY1, fScreenWidth, fScreenHeight, GetAspectRatio() * g_settings.m_fPixelRatio, g_settings.m_fZoomAmount);

	if(m_iActiveWidth == (rd.right - rd.left) && m_iActiveHeight == (rd.bottom - rd.top)
		&& m_iActivePosX == rd.left && m_iActivePosY == rd.top)
		return;

	m_iActiveWidth = rd.right - rd.left;
	m_iActiveHeight = rd.bottom - rd.top;
	m_iActivePosX = rd.left;
	m_iActivePosY = rd.top;

	COLORVERTEX Vertices[] =
	{
		{(FLOAT)rd.left, (FLOAT)rd.top,  0.0f, 0, 0 ,},
		{(FLOAT)rd.right, (FLOAT)rd.top, 0.0f,  1, 0 ,},
		{(FLOAT)rd.left, (FLOAT)rd.bottom, 0.0f,  0, 1 ,},
		{(FLOAT)rd.right, (FLOAT)rd.bottom, 0.0f,  1, 1 ,},
	};

	COLORVERTEX* pVertices;
	m_pVB->Lock(0, 0, ( void** )&pVertices, 0);
	memcpy(pVertices, Vertices, 4 * sizeof(COLORVERTEX));
	m_pVB->Unlock();
}

//***************************************************************************************
// CalculateFrameAspectRatio()
//
// Considers the source frame size and output frame size (as suggested by mplayer)
// to determine if the pixels in the source are not square.  It calculates the aspect
// ratio of the output frame.  We consider the cases of VCD, SVCD and DVD separately,
// as these are intended to be viewed on a non-square pixel TV set, so the pixels are
// defined to be the same ratio as the intended display pixels.
// These formats are determined by frame size.
//***************************************************************************************
void CRGBRenderer::CalculateFrameAspectRatio(int desired_width, int desired_height)
{
	m_fSourceFrameRatio = (float)desired_width / desired_height;

	// Check whether the size of the video file should be changed
	// This indicates either a scaling has taken place or it has
	// found an aspect ratio parameter from the file, and is changing
	// the frame size based on that.
	if (m_iSourceWidth == desired_width && m_iSourceHeight == desired_height)
		return;

	// Scaling in one or both directions. We must alter our Source Pixel Ratio
	float fImageFrameRatio = (float)m_iSourceWidth / m_iSourceHeight;

	// For sources intended for non-square pixel output (PAL/NTSC TVs)
	// including VCD, SVCD, and DVD
	float fPALPixelRatio = 128.0f / 117.0f;
	float fNTSCPixelRatio = 4320.0f / 4739.0f;

	// Calculate the correction needed for anamorphic sources
	float fNon4by3Correction = m_fSourceFrameRatio / (4.0f / 3.0f);

	// Check for VCD, SVCD or DVD frame sizes
	if (m_iSourceWidth == 352)
	{ // VCD?
		if (m_iSourceHeight == 240) // NTSC
			m_fSourceFrameRatio = fImageFrameRatio * fNTSCPixelRatio;
		if (m_iSourceHeight == 288) // PAL
			m_fSourceFrameRatio = fImageFrameRatio * fPALPixelRatio;
	}
	if (m_iSourceWidth == 480)
	{ // SVCD?
		if (m_iSourceHeight == 480) // NTSC
			m_fSourceFrameRatio = fImageFrameRatio * 3.0f / 2.0f * fNTSCPixelRatio * fNon4by3Correction;
		if (m_iSourceHeight == 576) // PAL
			m_fSourceFrameRatio = fImageFrameRatio * 3.0f / 2.0f * fPALPixelRatio * fNon4by3Correction;
	}
	if (m_iSourceWidth == 720)
	{ // DVD?
		if (m_iSourceHeight == 480) // NTSC
			m_fSourceFrameRatio = fImageFrameRatio * fNTSCPixelRatio * fNon4by3Correction;
		if (m_iSourceHeight == 576) // PAL
			m_fSourceFrameRatio = fImageFrameRatio * fPALPixelRatio * fNon4by3Correction;
	}
}

bool CRGBRenderer::Configure(int iWidth, int iHeight, int d_width, int d_height)
{
	// Release old textures if source dimensions have changed
	if(m_pFrameY && (m_iSourceWidth != iWidth || m_iSourceHeight != iHeight))
	{
		CLog::Log(LOGDEBUG, "CRGBRenderer::Configure - dimensions changed from %dx%d to %dx%d, releasing old textures",
			m_iSourceWidth, m_iSourceHeight, iWidth, iHeight);

		g_graphicsContext.TLock();
		// Unbind any textures the GPU may still be referencing
		m_pd3dDevice->SetTexture(0, NULL);
		m_pd3dDevice->SetTexture(1, NULL);
		m_pd3dDevice->SetTexture(2, NULL);
		g_graphicsContext.TUnlock();

		if(m_pFrameY) { g_graphicsContext.TLock(); m_pFrameY->Release(); g_graphicsContext.TUnlock(); m_pFrameY = NULL; }
		if(m_pFrameU) { g_graphicsContext.TLock(); m_pFrameU->Release(); g_graphicsContext.TUnlock(); m_pFrameU = NULL; }
		if(m_pFrameV) { g_graphicsContext.TLock(); m_pFrameV->Release(); g_graphicsContext.TUnlock(); m_pFrameV = NULL; }
	}

	m_iSourceWidth = iWidth;
	m_iSourceHeight = iHeight;

	// Calculate the input frame aspect ratio
	CalculateFrameAspectRatio(d_width, d_height);

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

	SetViewMode(g_settings.m_currentVideoSettings.m_ViewMode);
	ManageDisplay();

	return true;
}

bool CRGBRenderer::GetImage(YV12Image* image)
{
	if (!image) return false;
	if (!m_pFrameY || !m_pFrameU || !m_pFrameV) return false;

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
	if(m_pFrameU) m_pFrameU->UnlockRect(0);
	if(m_pFrameY) m_pFrameY->UnlockRect(0);
	if(m_pFrameV) m_pFrameV->UnlockRect(0);
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

void CRGBRenderer::CalcNormalDisplayRect(float fOffsetX1, float fOffsetY1, float fScreenWidth, float fScreenHeight, float fInputFrameRatio, float fZoomAmount)
{
	// scale up image as much as possible
	// and keep the aspect ratio (introduces with black bars)
	// calculate the correct output frame ratio (using the users pixel ratio setting
	// and the output pixel ratio setting)

	RESOLUTION iRes = g_graphicsContext.GetVideoResolution();
	float fOutputFrameRatio = fInputFrameRatio / g_settings.m_ResInfo[iRes].fPixelRatio;

	// allow a certain error to maximize screen size
	float fCorrection = fScreenWidth / fScreenHeight / fOutputFrameRatio - 1.0f;
	float fAllowedError = 0.03f; // 3% default (same as xbmc4xbox videoplayer.errorinaspect)
	if (fCorrection > fAllowedError)
		fCorrection = fAllowedError;
	if (fCorrection < -fAllowedError)
		fCorrection = -fAllowedError;

	fOutputFrameRatio *= 1.0f + fCorrection;

	// maximize the movie width
	float fNewWidth = fScreenWidth;
	float fNewHeight = fNewWidth / fOutputFrameRatio;

	if (fNewHeight > fScreenHeight)
	{
		fNewHeight = fScreenHeight;
		fNewWidth = fNewHeight * fOutputFrameRatio;
	}

	// Scale the movie up by set zoom amount
	fNewWidth *= fZoomAmount;
	fNewHeight *= fZoomAmount;

	// Centre the movie
	float fPosY = (fScreenHeight - fNewHeight) / 2;
	float fPosX = (fScreenWidth - fNewWidth) / 2;

	rd.left = (int)(fPosX + fOffsetX1);
	rd.right = (int)(rd.left + fNewWidth + 0.5f);
	rd.top = (int)(fPosY + fOffsetY1);
	rd.bottom = (int)(rd.top + fNewHeight + 0.5f);
}

void CRGBRenderer::SetViewMode(int iViewMode)
{
	if (iViewMode < VIEW_MODE_NORMAL || iViewMode > VIEW_MODE_CUSTOM)
		iViewMode = VIEW_MODE_NORMAL;

	g_settings.m_currentVideoSettings.m_ViewMode = iViewMode;

	if (g_settings.m_currentVideoSettings.m_ViewMode == VIEW_MODE_NORMAL)
	{
		g_settings.m_fPixelRatio = 1.0;
		g_settings.m_fZoomAmount = 1.0;
		return;
	}

	if (g_settings.m_currentVideoSettings.m_ViewMode == VIEW_MODE_CUSTOM)
	{
		g_settings.m_fZoomAmount = g_settings.m_currentVideoSettings.m_CustomZoomAmount;
		g_settings.m_fPixelRatio = g_settings.m_currentVideoSettings.m_CustomPixelRatio;
		return;
	}

	// get our calibrated full screen resolution
	RESOLUTION iRes = g_graphicsContext.GetVideoResolution();
	float fOffsetX1 = (float)g_settings.m_ResInfo[iRes].Overscan.left;
	float fOffsetY1 = (float)g_settings.m_ResInfo[iRes].Overscan.top;
	float fScreenWidth = (float)(g_settings.m_ResInfo[iRes].Overscan.right - g_settings.m_ResInfo[iRes].Overscan.left);
	float fScreenHeight = (float)(g_settings.m_ResInfo[iRes].Overscan.bottom - g_settings.m_ResInfo[iRes].Overscan.top);

	// and the source frame ratio
	float fSourceFrameRatio = GetAspectRatio();

	if (g_settings.m_currentVideoSettings.m_ViewMode == VIEW_MODE_ZOOM)
	{ // zoom image so no black bars
		g_settings.m_fPixelRatio = 1.0;
		// calculate the desired output ratio
		float fOutputFrameRatio = fSourceFrameRatio * g_settings.m_fPixelRatio / g_settings.m_ResInfo[iRes].fPixelRatio;
		// now calculate the correct zoom amount.  First zoom to full height.
		float fNewHeight = fScreenHeight;
		float fNewWidth = fNewHeight * fOutputFrameRatio;
		g_settings.m_fZoomAmount = fNewWidth / fScreenWidth;
		if (fNewWidth < fScreenWidth)
		{ // zoom to full width
			fNewWidth = fScreenWidth;
			fNewHeight = fNewWidth / fOutputFrameRatio;
			g_settings.m_fZoomAmount = fNewHeight / fScreenHeight;
		}
	}
	else if (g_settings.m_currentVideoSettings.m_ViewMode == VIEW_MODE_STRETCH_4x3)
	{ // stretch image to 4:3 ratio
		g_settings.m_fZoomAmount = 1.0;
		if (iRes == PAL_4x3 || iRes == PAL60_4x3 || iRes == NTSC_4x3 || iRes == HDTV_480p_4x3)
		{ // stretch to the limits of the 4:3 screen.
			g_settings.m_fPixelRatio = (fScreenWidth / fScreenHeight) * g_settings.m_ResInfo[iRes].fPixelRatio / fSourceFrameRatio;
		}
		else
		{
			// now we need to set g_settings.m_fPixelRatio so that
			// fOutputFrameRatio = 4:3.
			g_settings.m_fPixelRatio = (4.0f / 3.0f) / fSourceFrameRatio;
		}
	}
	else if (g_settings.m_currentVideoSettings.m_ViewMode == VIEW_MODE_STRETCH_14x9)
	{ // stretch image to 14:9 ratio
		// now we need to set g_settings.m_fPixelRatio so that
		// fOutputFrameRatio = 14:9.
		g_settings.m_fPixelRatio = (14.0f / 9.0f) / fSourceFrameRatio;
		// calculate the desired output ratio
		float fOutputFrameRatio = fSourceFrameRatio * g_settings.m_fPixelRatio / g_settings.m_ResInfo[iRes].fPixelRatio;
		// now calculate the correct zoom amount.  First zoom to full height.
		float fNewHeight = fScreenHeight;
		float fNewWidth = fNewHeight * fOutputFrameRatio;
		g_settings.m_fZoomAmount = fNewWidth / fScreenWidth;
		if (fNewWidth < fScreenWidth)
		{ // zoom to full width
			fNewWidth = fScreenWidth;
			fNewHeight = fNewWidth / fOutputFrameRatio;
			g_settings.m_fZoomAmount = fNewHeight / fScreenHeight;
		}
	}
	else if (g_settings.m_currentVideoSettings.m_ViewMode == VIEW_MODE_STRETCH_16x9)
	{ // stretch image to 16:9 ratio
		g_settings.m_fZoomAmount = 1.0;
		if (iRes == PAL_4x3 || iRes == PAL60_4x3 || iRes == NTSC_4x3 || iRes == HDTV_480p_4x3)
		{ // now we need to set g_settings.m_fPixelRatio so that
			// fOutputFrameRatio = 16:9.
			g_settings.m_fPixelRatio = (16.0f / 9.0f) / fSourceFrameRatio;
		}
		else
		{ // stretch to the limits of the 16:9 screen.
			g_settings.m_fPixelRatio = (fScreenWidth / fScreenHeight) * g_settings.m_ResInfo[iRes].fPixelRatio / fSourceFrameRatio;
		}
	}
	else // if (g_settings.m_currentVideoSettings.m_ViewMode == VIEW_MODE_ORIGINAL)
	{ // zoom image so that the height is the original size
		g_settings.m_fPixelRatio = 1.0;
		// calculate the desired output ratio
		float fOutputFrameRatio = fSourceFrameRatio * g_settings.m_fPixelRatio / g_settings.m_ResInfo[iRes].fPixelRatio;
		// now calculate the correct zoom amount.  First zoom to full width.
		float fNewWidth = fScreenWidth;
		float fNewHeight = fNewWidth / fOutputFrameRatio;
		if (fNewHeight > fScreenHeight)
		{ // zoom to full height
			fNewHeight = fScreenHeight;
			fNewWidth = fNewHeight * fOutputFrameRatio;
		}
		// now work out the zoom amount so that no zoom is done
		g_settings.m_fZoomAmount = (float)m_iSourceHeight / fNewHeight;
	}
}

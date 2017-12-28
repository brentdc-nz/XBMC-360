#include "player.h"
#include "shader.h"

//-------------------------------------------------------------------------------------
// Structure to hold vertex data.
//-------------------------------------------------------------------------------------
struct COLORVERTEX
{
	float       Position[3];
	float       Coor[2];
};

//-------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------
D3DDevice*             g_pd3dDevice;    // Our rendering device
D3DVertexBuffer*       g_pVB;           // Buffer to hold vertices
D3DVertexDeclaration*  g_pVertexDecl;   // Vertex format decl
D3DVertexShader*       g_pVertexShader; // Vertex Shader
D3DPixelShader*        g_pPixelShader;  // Pixel Shader

//-------------------------------------------------------------------------------------
// Surface displayed on screen (updated each frame)
//-------------------------------------------------------------------------------------
IDirect3DTexture9 * g_pFrameU = NULL;
IDirect3DTexture9 * g_pFrameV = NULL;
IDirect3DTexture9 * g_pFrameY = NULL;

//-------------------------------------------------------------------------------------
// View matrice
//-------------------------------------------------------------------------------------
XMMATRIX g_matWorld;
XMMATRIX g_matProj;
XMMATRIX g_matView;
XMMATRIX g_matScale;

FLOAT fAspect = 1.0f; 
BOOL g_bWidescreen = TRUE;

float g_xbWidth,g_xbHeight;

HRESULT InitD3D()
{
	// Create the D3D object.
	Direct3D* pD3D = Direct3DCreate9( D3D_SDK_VERSION );
	if( !pD3D )
		return E_FAIL;

	// Set up the structure used to create the D3DDevice.
	D3DPRESENT_PARAMETERS d3dpp; 
	ZeroMemory( &d3dpp, sizeof(d3dpp) );
	XVIDEO_MODE VideoMode;
	XGetVideoMode( &VideoMode );
	g_bWidescreen = VideoMode.fIsWideScreen;
	d3dpp.BackBufferWidth        = min( VideoMode.dwDisplayWidth, 1280 );
	d3dpp.BackBufferHeight       = min( VideoMode.dwDisplayHeight, 720 );
	d3dpp.BackBufferFormat       = D3DFMT_X8R8G8B8;
	d3dpp.BackBufferCount        = 1;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
	d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;

	//Setup scale ratio
	g_xbWidth = (float)d3dpp.BackBufferWidth;
	g_xbHeight = (float)d3dpp.BackBufferHeight;

	// Create the Direct3D device.
	if( FAILED( pD3D->CreateDevice( 0, D3DDEVTYPE_HAL, NULL,
		//D3DCREATE_BUFFER_2_FRAMES|D3DCREATE_CREATE_THREAD_ON_0,
		0,
		&d3dpp, &g_pd3dDevice ) ) )
		return E_FAIL;

	return S_OK;
}

HRESULT InitScene()
{
	// Compile vertex shader.
	ID3DXBuffer* pVertexShaderCode;
	ID3DXBuffer* pVertexErrorMsg;
	HRESULT hr = D3DXCompileShader( g_strVertexShaderProgram, 
		(UINT)strlen( g_strVertexShaderProgram ),
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
		return E_FAIL;
	}    

	// Create vertex shader.
	g_pd3dDevice->CreateVertexShader( (DWORD*)pVertexShaderCode->GetBufferPointer(), 
		&g_pVertexShader );

	// Compile pixel shader.
	ID3DXBuffer* pPixelShaderCode;
	ID3DXBuffer* pPixelErrorMsg;
	hr = D3DXCompileShader( g_strPixelShaderProgram, 
		(UINT)strlen( g_strPixelShaderProgram ),
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
		return E_FAIL;
	}

	// Create pixel shader.
	g_pd3dDevice->CreatePixelShader( (DWORD*)pPixelShaderCode->GetBufferPointer(), 
		&g_pPixelShader );

	// Define the vertex elements and
	// Create a vertex declaration from the element descriptions.
	D3DVERTEXELEMENT9 VertexElements[3] =
	{
		{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};
	g_pd3dDevice->CreateVertexDeclaration( VertexElements, &g_pVertexDecl );

	// Create the vertex buffer. Here we are allocating enough memory
	// (from the default pool) to hold all our 3 custom vertices. 
	if( FAILED( g_pd3dDevice->CreateVertexBuffer( 4*sizeof(COLORVERTEX),
		D3DUSAGE_WRITEONLY, 
		NULL,
		D3DPOOL_MANAGED, 
		&g_pVB, 
		NULL ) ) )
		return E_FAIL;

	// Now we fill the vertex buffer. To do this, we need to Lock() the VB to
	// gain access to the vertices. This mechanism is required because the
	// vertex buffer may still be in use by the GPU. This can happen if the
	// CPU gets ahead of the GPU. The GPU could still be rendering the previous
	// frame.
	COLORVERTEX g_Vertices[] =
	{
		//square
		{ -1.0f, -1.0f, 0.0f,  0.0f,  1.0f },//1
		{ -1.0f,  1.0f, 0.0f,  0.0f,  0.0f },//2
		{  1.0f,  1.0f, 0.0f,  1.0f,  0.0f },//4
		{  1.0f, -1.0f, 0.0f,  1.0f,  1.0f }//3
	};

	COLORVERTEX* pVertices;
	if( FAILED( g_pVB->Lock( 0, 0, (void**)&pVertices, 0 ) ) )
		return E_FAIL;
	memcpy( pVertices, g_Vertices, 4*sizeof(COLORVERTEX) );
	g_pVB->Unlock();

	// Initialize the world matrix
	g_matWorld = XMMatrixIdentity();

	// Initialize the projection matrix
	//FLOAT fAspect = ( g_bWidescreen ) ? (16.0f / 9.0f) : (4.0f / 3.0f); 
	FLOAT fAspect = 1.0f; 
	g_matProj = XMMatrixPerspectiveFovLH( XM_PIDIV4, fAspect, 1.0f, 200.0f );

	// Initialize the view matrix
	XMVECTOR vEyePt    = { 0.0f, 0.0f,-2.4f, 0.0f };
	XMVECTOR vLookatPt = { 0.0f, 0.0f, 0.0f, 0.0f };
	XMVECTOR vUp       = { 0.0f, 1.0f, 0.0f, 0.0f };
	g_matView = XMMatrixLookAtLH( vEyePt, vLookatPt, vUp );
	g_matProj = XMMatrixPerspectiveFovLH( XM_PIDIV4, fAspect, 1.0f, 200.0f );


	//Set Sampler State
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );

	g_pd3dDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	g_pd3dDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	g_pd3dDevice->SetSamplerState( 1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );

	g_pd3dDevice->SetSamplerState( 2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	g_pd3dDevice->SetSamplerState( 2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	g_pd3dDevice->SetSamplerState( 2, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );

	// Set Shaders
	g_pd3dDevice->SetVertexDeclaration( g_pVertexDecl );
	g_pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof(COLORVERTEX) );
	g_pd3dDevice->SetVertexShader( g_pVertexShader );
	g_pd3dDevice->SetPixelShader( g_pPixelShader );

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: EnableOverlays()
// Desc: Enable the overlay planes for playing the movie in them, and record
// that the overlays should be disabled when Destroy() is called.
//-----------------------------------------------------------------------------
void EnableOverlays( LPDIRECT3DDEVICE8 pDevice )
{
    pDevice->EnableOverlay( TRUE );
}

void Render()
{
	EnableOverlays(g_pd3dDevice);
	

	// Clear the backbuffer to a blue color
	g_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, 
		D3DCOLOR_XRGB(0,0,0), 1.0f, 0L );

	// set YUV texture ...
	g_pd3dDevice->SetTexture(0,g_pFrameY);
	g_pd3dDevice->SetTexture(1,g_pFrameU);
	g_pd3dDevice->SetTexture(2,g_pFrameV);

	// Build the world-view-projection matrix and pass it into the vertex shader
	XMMATRIX matWVP = g_matWorld * g_matView * g_matProj * g_matScale;
	g_pd3dDevice->SetVertexShaderConstantF( 0, (FLOAT*)&matWVP, 4 );

	// Draw the vertices in the vertex buffer
	g_pd3dDevice->DrawPrimitive( D3DPT_RECTLIST, 0,1 );

	// Present the backbuffer contents to the display
	g_pd3dDevice->Present( NULL, NULL, NULL, NULL );

	g_pd3dDevice->SetTexture(0,NULL);
	g_pd3dDevice->SetTexture(1,NULL);
	g_pd3dDevice->SetTexture(2,NULL);
}

HRESULT InitYuvSurface(
	D3DTexture **pFrameY,D3DTexture **pFrameU,D3DTexture **pFrameV,
	int width, int height)
{
	//1/((720)/(800*1280/1980))
	g_matScale = XMMatrixScaling(1.f, 
		1/(
			(float)g_xbHeight/(
				(float)height*((float)g_xbWidth/width)
			)
		)
	, 1.f);

	D3DXCreateTexture(
		g_pd3dDevice, width,
		height, D3DX_DEFAULT, 0, 
		D3DFMT_LIN_L8, D3DPOOL_MANAGED,
		pFrameY
	);
	D3DXCreateTexture(
		g_pd3dDevice, width/2,
		height/2, D3DX_DEFAULT, 0, 
		D3DFMT_LIN_L8, D3DPOOL_MANAGED,
		pFrameU
	);
	D3DXCreateTexture(
		g_pd3dDevice, width/2,
		height/2, D3DX_DEFAULT, 0, 
		D3DFMT_LIN_L8, D3DPOOL_MANAGED,
		pFrameV
	);

	if(pFrameV==NULL || pFrameU==NULL || pFrameY == NULL){
		return E_FAIL;
	}
	return S_OK;
};

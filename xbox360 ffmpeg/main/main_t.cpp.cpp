// main.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#define UINT64_C(val) val##ui64
extern "C"{
#include <avcodec.h>
#include <avformat.h>
#include <swscale.h>
}
#include <xaudio2.h>

#undef printf
#undef fprintf
#undef malloc
#undef free

#include <stdio.h>
#include "shader.h"

#define SAFE_DELETE(a) if(a!=NULL){ delete (a); (a) = NULL; }
#define SAFE_FREE(a) if(a!=NULL){ free(a); (a) = NULL; }
#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

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

IDirect3DTexture9 * g_pFrameU = NULL;
IDirect3DTexture9 * g_pFrameV = NULL;
IDirect3DTexture9 * g_pFrameY = NULL;

XMMATRIX g_matWorld;
XMMATRIX g_matProj;
XMMATRIX g_matView;

BOOL g_bWidescreen = TRUE;

HRESULT video_ffmepg(char *);


//-------------------------------------------------------------------------------------
// 	Audio Synchronization
//-------------------------------------------------------------------------------------
class XAudio2_BufferNotify : public IXAudio2VoiceCallback
{
public:
	HANDLE hBufferEndEvent;

	XAudio2_BufferNotify() {
		hBufferEndEvent = NULL;
		hBufferEndEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	}

	~XAudio2_BufferNotify() {
		CloseHandle( hBufferEndEvent );
		hBufferEndEvent = NULL;
	}

	STDMETHOD_( void, OnBufferEnd ) ( void *pBufferContext ) {
		SetEvent( hBufferEndEvent );
		if(pBufferContext)
			free(pBufferContext);
	}

	// dummies:
	STDMETHOD_( void, OnVoiceProcessingPassStart ) ( UINT32 BytesRequired ) {}
	STDMETHOD_( void, OnVoiceProcessingPassEnd ) () {}
	STDMETHOD_( void, OnStreamEnd ) () {}
	STDMETHOD_( void, OnBufferStart ) ( void *pBufferContext ) {}
	STDMETHOD_( void, OnLoopEnd ) ( void *pBufferContext ) {}
	STDMETHOD_( void, OnVoiceError ) ( void *pBufferContext, HRESULT Error ) {};
};
XAudio2_BufferNotify XAudio2_Notifier; //XAudio2 event notifier

//-------------------------------------------------------------------------------------
// Name: InitD3D()
// Desc: Initializes Direct3D
//-------------------------------------------------------------------------------------
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
	d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_ONE;

	// Create the Direct3D device.
	if( FAILED( pD3D->CreateDevice( 0, D3DDEVTYPE_HAL, NULL,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&d3dpp, &g_pd3dDevice ) ) )
		return E_FAIL;

	return S_OK;
}


//-------------------------------------------------------------------------------------
// Name: InitScene()
// Desc: Creates the scene.  First we compile our shaders. For the final version
//       of a game, you should store the shaders in binary form; don't call 
//       D3DXCompileShader at runtime. Next, we declare the format of our 
//       vertices, and then create a vertex buffer. The vertex buffer is basically
//       just a chunk of memory that holds vertices. After creating it, we must 
//       Lock()/Unlock() it to fill it. Finally, we set up our world, projection,
//       and view matrices.
//-------------------------------------------------------------------------------------
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
	XMVECTOR vEyePt    = { 0.0f, 0.0f,-3.0f, 0.0f };
	XMVECTOR vLookatPt = { 0.0f, 0.0f, 0.0f, 0.0f };
	XMVECTOR vUp       = { 0.0f, 1.0f, 0.0f, 0.0f };
	g_matView = XMMatrixLookAtLH( vEyePt, vLookatPt, vUp );

	return S_OK;
}

//-------------------------------------------------------------------------------------
// Name: Update()
// Desc: Updates the world for the next frame
//-------------------------------------------------------------------------------------
void Update()
{
	// Set the world matrix
	// float fAngle = fmodf( -g_Time.fAppTime, XM_2PI );
	// static const XMVECTOR vAxisZ = { 0, 1.0f, 0.0f, 0 };
	// g_matWorld = XMMatrixRotationAxis( vAxisZ, fAngle );
}


//-------------------------------------------------------------------------------------
// Name: Render()
// Desc: Draws the scene
//-------------------------------------------------------------------------------------
void Render()
{
	// Clear the backbuffer to a blue color
	g_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, 
		D3DCOLOR_XRGB(0,0,255), 1.0f, 0L );

	// Draw the triangles in the vertex buffer. This is broken into a few steps:

	// We are passing the vertices down a "stream", so first we need
	// to specify the source of that stream, which is our vertex buffer. 
	// Then we need to let D3D know what vertex and pixel shaders to use. 
	g_pd3dDevice->SetVertexDeclaration( g_pVertexDecl );
	g_pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof(COLORVERTEX) );
	g_pd3dDevice->SetVertexShader( g_pVertexShader );
	g_pd3dDevice->SetPixelShader( g_pPixelShader );

	// set YUV texture ...
	g_pd3dDevice->SetTexture(0,g_pFrameY);
	g_pd3dDevice->SetTexture(1,g_pFrameU);
	g_pd3dDevice->SetTexture(2,g_pFrameV);

	// Build the world-view-projection matrix and pass it into the vertex shader
	XMMATRIX matWVP = g_matWorld * g_matView * g_matProj;
	g_pd3dDevice->SetVertexShaderConstantF( 0, (FLOAT*)&matWVP, 4 );

	// Draw the vertices in the vertex buffer
	g_pd3dDevice->DrawPrimitive( D3DPT_RECTLIST, 0,1 );

	// Present the backbuffer contents to the display
	g_pd3dDevice->Present( NULL, NULL, NULL, NULL );

	g_pd3dDevice->SetTexture(0,NULL);
	g_pd3dDevice->SetTexture(1,NULL);
	g_pd3dDevice->SetTexture(2,NULL);
}

void InitYuvSurface(
	D3DTexture **pFrameY,D3DTexture **pFrameU,D3DTexture **pFrameV,
	int width, int height
){

	D3DXCreateTexture(
		g_pd3dDevice, width,
		height, D3DX_DEFAULT, 0, D3DFMT_LIN_L8, D3DPOOL_MANAGED,
		pFrameY
		);
	D3DXCreateTexture(
		g_pd3dDevice, width/2,
		height/2, D3DX_DEFAULT, 0, D3DFMT_LIN_L8, D3DPOOL_MANAGED,
		pFrameU
		);
	D3DXCreateTexture(
		g_pd3dDevice, width/2,
		height/2, D3DX_DEFAULT, 0, D3DFMT_LIN_L8, D3DPOOL_MANAGED,
		pFrameV
		);

};

//-------------------------------------------------------------------------------------
// Name: main()
// Desc: The application's entry point
//-------------------------------------------------------------------------------------
void __cdecl main()
{
	// Initialize Direct3D
	if( FAILED( InitD3D() ) )
		return;

	// Initialize the vertex buffer
	if( FAILED( InitScene() ) )
		return;

	video_ffmepg("game:\\video.avi");
}

void displayFrame(AVPicture * pict,int w, int h){

	RECT d3dr;
	d3dr.left=0;
	d3dr.top=0;
	d3dr.right=w;
	d3dr.bottom=h;

	// Update the world
	Update();   
	// Render the scene
	Render();
}

#include <xaudio2.h>

#define FFMPEG_VIDEO_BUFFER 100

typedef struct DxYUV{
	IDirect3DTexture9 * pFrameU;
	IDirect3DTexture9 * pFrameV;
	IDirect3DTexture9 * pFrameY;
} DxYUV;

typedef struct FFMpegVideoFrame {
    int64_t timestamp;
	DxYUV pYuv;
    int filled;
} FFMpegVideoFrame;

typedef struct FFMpegData {
	//Fomat
	AVFormatContext *pFormatCtx;

	//Video
	AVCodecContext  *pVideoCodecCtx;
	AVCodec         *pVideoCodec;

	//Audio
	AVCodecContext  *pAudioCodecCtx;
	AVCodec			*pAudioCodec;

	//Scale
	SwsContext * pSWSContext;

	//Decoding stuff ...
	AVFrame         *pFrame; 
	AVPacket        packet;

	//Stream ..
	int videoStreamNumber;
	int audioStreamNumber;

	//Mutex
	HANDLE hDecodeSemaphore;
	HANDLE hVideoMutex;
	HANDLE hAudioMutex;

	//Buffer
	FFMpegVideoFrame videoBuffer[FFMPEG_VIDEO_BUFFER];

} FFMpegData, *PFFMpegData;

//FFMpegData g_FFMpegData;

//-------------------------------------------------------------------------------------
// Audio data ..
//-------------------------------------------------------------------------------------	
AVPacket   g_AudioPacket;

//-------------------------------------------------------------------------------------
// Xaudio2 things ...
//-------------------------------------------------------------------------------------	
IXAudio2* g_pXAudio2 = NULL;
IXAudio2MasteringVoice* g_pMasteringVoice = NULL;
IXAudio2SourceVoice* g_pSourceVoice = NULL;

XAUDIO2_BUFFER g_SoundBuffer;
WAVEFORMATEX * g_pWfx  = NULL;

BYTE *  AudioBytes;

HRESULT InitAudio(FFMpegData * pFFMpegData){
	if(pFFMpegData->pAudioCodecCtx==NULL){
		OutputDebugStringA( "pAudioCodecCtx = NULL\n");
		return E_FAIL;
	}

	//-------------------------------------------------------------------------------------
	// Initialise Audio
	//-------------------------------------------------------------------------------------	
	HRESULT hr;
	if( FAILED( hr = XAudio2Create( &g_pXAudio2, 0 ) ) )
	{
		OutputDebugStringA( "Failed to init XAudio2 engine\n");
		return E_FAIL;
	}

	//-------------------------------------------------------------------------------------
	// Create a mastering voice
	//-------------------------------------------------------------------------------------	
	if( FAILED( hr = g_pXAudio2->CreateMasteringVoice( &g_pMasteringVoice ) ) )
	{
		OutputDebugStringA( "Failed creating mastering voice\n");
		return E_FAIL;
	}
	//-------------------------------------------------------------------------------------
	// Create source voice
	//-------------------------------------------------------------------------------------	
	WAVEFORMATEXTENSIBLE wfx;
	memset(&wfx, 0, sizeof(WAVEFORMATEXTENSIBLE));

	wfx.Format.wFormatTag           = WAVE_FORMAT_EXTENSIBLE ;
	wfx.Format.nSamplesPerSec       = pFFMpegData->pAudioCodecCtx->sample_rate;//48000 by default
	wfx.Format.nChannels            = pFFMpegData->pAudioCodecCtx->channels;
	wfx.Format.wBitsPerSample       = 16;
	wfx.Format.nBlockAlign          = wfx.Format.nChannels*16/8;
	wfx.Format.nAvgBytesPerSec      = wfx.Format.nSamplesPerSec * wfx.Format.nBlockAlign;
	wfx.Format.cbSize               = sizeof(WAVEFORMATEXTENSIBLE)-sizeof(WAVEFORMATEX);
	wfx.Samples.wValidBitsPerSample = wfx.Format.wBitsPerSample;
	wfx.dwChannelMask               = SPEAKER_STEREO;
	wfx.SubFormat                   = KSDATAFORMAT_SUBTYPE_PCM;

	//-------------------------------------------------------------------------------------
	//	Source voice
	//-------------------------------------------------------------------------------------
	if(FAILED( g_pXAudio2->CreateSourceVoice(&g_pSourceVoice,(WAVEFORMATEX*)&wfx, XAUDIO2_VOICE_NOSRC | XAUDIO2_VOICE_NOPITCH , 1.0f, &XAudio2_Notifier)	))
	{
		OutputDebugStringA("CreateSourceVoice failed\n");
		return E_FAIL;
	}

	//-------------------------------------------------------------------------------------
	// Start sound
	//-------------------------------------------------------------------------------------	
	if ( FAILED(g_pSourceVoice->Start( 0 ) ) )
	{
		OutputDebugStringA("g_pSourceVoice failed\n");
		return E_FAIL;
	}

	return S_OK;
}

HRESULT UpdateAudio(){
	return S_OK;
}

FFMpegVideoFrame * GetVideoFrame(FFMpegData * data){
	FFMpegVideoFrame * pFrame = NULL;//Error ...

	int i=0;
	while(i<FFMPEG_VIDEO_BUFFER){
		if(data->videoBuffer[i].filled==1){
			data->videoBuffer[i].filled=0;
			//return &data->videoBuffer[i];
			break;
		}
		i++;

		if(i==FFMPEG_VIDEO_BUFFER){
			return NULL;
		}
	}
	return &data->videoBuffer[i];
};


HRESULT AddFrameToQueue(
	FFMpegData * data
){
	//Find a place ...
	int i=0;
	WaitForSingleObject(data->hVideoMutex,INFINITE);
	while(1){
		if(data->videoBuffer[i].filled==0){
			break;
		}
		i++;

		if(i==FFMPEG_VIDEO_BUFFER){
			i=0;
			Sleep(10);//Wait 10msec... restart...
		}
	}

	data->videoBuffer[i].filled = 1;

	AVPicture pict;
	memset(&pict, 0, sizeof(pict));

	InitYuvSurface(
		&data->videoBuffer[i].pYuv.pFrameY,
		&data->videoBuffer[i].pYuv.pFrameU,
		&data->videoBuffer[i].pYuv.pFrameV,
		data->pVideoCodecCtx->width,
		data->pVideoCodecCtx->height
	);

	//Lock texture to access data
	D3DLOCKED_RECT lockRectY;
	D3DLOCKED_RECT lockRectU;
	D3DLOCKED_RECT lockRectV;

	data->videoBuffer[i].pYuv.pFrameY->LockRect( 0, &lockRectY, NULL, 0 );
	data->videoBuffer[i].pYuv.pFrameU->LockRect( 0, &lockRectU, NULL, 0 );
	data->videoBuffer[i].pYuv.pFrameV->LockRect( 0, &lockRectV, NULL, 0 );

	pict.data[2] = (uint8_t*)lockRectY.pBits;
	pict.data[1] = (uint8_t*)lockRectU.pBits;
	pict.data[0] = (uint8_t*)lockRectV.pBits;

	pict.linesize[2] = lockRectY.Pitch;
	pict.linesize[1] = lockRectU.Pitch;
	pict.linesize[0] = lockRectV.Pitch;

	//Scale it ..
	sws_scale(
		data->pSWSContext,  data->pFrame->data, 
		data->pFrame->linesize, 0, 
		data->pVideoCodecCtx->height, 
		pict.data, pict.linesize
	);

	//Release lock on texture
	data->videoBuffer[i].pYuv.pFrameY->UnlockRect(0);
	data->videoBuffer[i].pYuv.pFrameU->UnlockRect(0);
	data->videoBuffer[i].pYuv.pFrameV->UnlockRect(0);

	//Unlock
	ReleaseMutex(data->hVideoMutex);

	return S_OK;
}
/*
HRESULT UpdateFrame(SwsContext *pCtx, AVFrame * pFrame, int width, int height){
	if(pCtx==NULL){
		return E_FAIL;
	}
	if(pFrame==NULL){
		return E_FAIL;
	}

	AVPicture pict;
	memset(&pict, 0, sizeof(pict));

	//Lock texture to access data
	D3DLOCKED_RECT lockRectY;
	D3DLOCKED_RECT lockRectU;
	D3DLOCKED_RECT lockRectV;

	g_pFrameU->LockRect( 0, &lockRectY, NULL, 0 );
	g_pFrameV->LockRect( 0, &lockRectU, NULL, 0 );
	g_pFrameY->LockRect( 0, &lockRectV, NULL, 0 );

	pict.data[2] = (uint8_t*)lockRectY.pBits;
	pict.data[1] = (uint8_t*)lockRectU.pBits;
	pict.data[0] = (uint8_t*)lockRectV.pBits;

	pict.linesize[2] = lockRectY.Pitch;
	pict.linesize[1] = lockRectU.Pitch;
	pict.linesize[0] = lockRectV.Pitch;

	//Scale it ..
	sws_scale(pCtx, pFrame->data, pFrame->linesize, 0, height, pict.data, pict.linesize);

	//Release lock
	g_pFrameU->UnlockRect(0);
	g_pFrameY->UnlockRect(0);
	g_pFrameV->UnlockRect(0);

	displayFrame(&pict,width, height);

	return S_OK;
}
*/

DWORD WINAPI FfmpegDecodeThread( LPVOID lpParam );

DWORD dwDecodeThreadRunning = 1;

HRESULT InitThread(FFMpegData * data){

	data->hDecodeSemaphore = CreateSemaphore( 
        NULL,           // default security attributes
        1,				// initial count
        10,  // maximum count
        NULL			// unnamed semaphore
	);

	data->hVideoMutex = CreateMutex( 
		NULL,FALSE,NULL
	);

	//Not used yet
	data->hAudioMutex = CreateMutex( 
		NULL,FALSE,NULL
	);

	for(int i=0;i<FFMPEG_VIDEO_BUFFER;i++){
		data->videoBuffer[i].filled=0;
	}

	DWORD dwThreadId = 0;
	HANDLE hThread = CreateThread(NULL, 0, FfmpegDecodeThread, data, CREATE_SUSPENDED, &dwThreadId);
	XSetThreadProcessor(hThread,3);
	ResumeThread(hThread);
	
	if(hThread)
		return S_OK;
	else
		return E_FAIL;
};

DWORD WINAPI FfmpegDecodeThread( LPVOID lpParam ){
	if(lpParam==NULL)
		DebugBreak();

	FFMpegData * FFData = (FFMpegData *)lpParam;

	printf_s("%s\n",FFData->pFormatCtx->filename);

	int frameFinished = 0;
	int len = 0;

	while(dwDecodeThreadRunning){
		//-------------------------------------------------------------------------------------
		// 	Main loop
		//-------------------------------------------------------------------------------------	

		//On as le droit ?
		//WaitForSingleObject(FFData->hDecodeSemaphore,INFINITE);
		int decode = av_read_frame(FFData->pFormatCtx, &FFData->packet);
		//Release the semaphore ...
		//ReleaseSemaphore(FFData->hDecodeSemaphore, 1, NULL);

		/* if we did not get a packet, we wait a bit and try again */
        if(decode < 0) {
            /* thread is idle */
			//Sleep(10);
            continue;
        }
		
		//we got a packet !!
		if(1) {
			//-------------------------------------------------------------------------------------
			// 	Handle video stream
			//-------------------------------------------------------------------------------------	
			if(FFData->packet.stream_index==FFData->videoStreamNumber) {
				// Decode video frame
				int len = avcodec_decode_video(
					FFData->pVideoCodecCtx, 
					FFData->pFrame, 
					&frameFinished, 
					FFData->packet.data, 
					FFData->packet.size
				);
				if(len<0)
				{
					OutputDebugString("Error in decoding video\n");
					av_free_packet(&FFData->packet);
					continue;
				}
				// Did we get a video frame?
				if(frameFinished) {
					AddFrameToQueue(
						FFData
					);
				}
			}
			//-------------------------------------------------------------------------------------
			// 	Handle Audio stream
			//-------------------------------------------------------------------------------------	
			else if(FFData->packet.stream_index==FFData->audioStreamNumber){
				
				continue;

				int done=AVCODEC_MAX_AUDIO_FRAME_SIZE;

				if (avcodec_decode_audio2(
					FFData->pAudioCodecCtx,
					(int16_t *)AudioBytes,
					&done,
					FFData->packet.data,FFData->packet.size
					) < 0 ) 
				{
					OutputDebugString("Error in decoding audio\n");
					av_free_packet(&FFData->packet);
					continue;
				}
				//if (done) {
				if(0){
					//Create a copie for some sound issue ...
					//freeed by callback
					BYTE * sound = (BYTE *)malloc(done * sizeof(BYTE));
					XMemCpy(sound,AudioBytes,done* sizeof(BYTE));

					g_SoundBuffer.AudioBytes=done;
					g_SoundBuffer.pAudioData=sound;
					g_SoundBuffer.pContext=(void*)g_SoundBuffer.pAudioData;
					g_SoundBuffer.Flags = XAUDIO2_END_OF_STREAM;

					//-------------------------------------------------------------------------------------
					// Send sound stream
					//-------------------------------------------------------------------------------------	
					if( FAILED(g_pSourceVoice->SubmitSourceBuffer( &g_SoundBuffer ) ) )
					{
						OutputDebugStringA("SubmitSourceBuffer failed\n");
					}
				}
			}
		}
		Sleep(40);
		//Wait 4ms ..
		// Free the packet that was allocated by av_read_frame
		av_free_packet(&FFData->packet);
	}

	return S_OK;

};


HRESULT video_ffmepg(char * v)
{
	int             i;
	int             numBytes;

	FFMpegData * pFFMpegData = (FFMpegData *) malloc(sizeof(FFMpegData));

	//Zero it ...
	memset(pFFMpegData,0,sizeof(FFMpegData));

	//-------------------------------------------------------------------------------------
	// Register ffmpeg formats/codecs etc ..
	//-------------------------------------------------------------------------------------	
	av_register_all();

	//-------------------------------------------------------------------------------------
	// Open and get steam information
	//-------------------------------------------------------------------------------------	
	if(av_open_input_file(&pFFMpegData->pFormatCtx, v, NULL, 0, NULL)!=0)
	{
		OutputDebugStringA("can't open file for reading\n");
		return E_FAIL; // Couldn't open file
	}

	// Retrieve stream information
	if(av_find_stream_info(pFFMpegData->pFormatCtx)<0){
		OutputDebugStringA("Can't fetch info from file\n");
		return E_FAIL; // Couldn't find stream information
	}

	// Dump information about file onto standard error
#if DEBUG
	dump_format(pFFMpegData->pFormatCtx, 0, v, 0);
#endif

	//-------------------------------------------------------------------------------------
	// Find stream
	//-------------------------------------------------------------------------------------	
	for(i=0; i<pFFMpegData->pFormatCtx->nb_streams; i++){
		if(pFFMpegData->pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
			pFFMpegData->videoStreamNumber=i;
		}
		else if(pFFMpegData->pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO) {
			pFFMpegData->audioStreamNumber=i;
		}
	}

	// if we don't have video ... why play it ?
	if(pFFMpegData->videoStreamNumber==-1)
	{
		OutputDebugStringA("Didn't find a video stream!\n");
		return E_FAIL; // Didn't find a video stream
	}

	//-------------------------------------------------------------------------------------
	// 	Read audio codec
	//-------------------------------------------------------------------------------------	
	if(pFFMpegData->audioStreamNumber>=0)
	{
		pFFMpegData->pAudioCodecCtx=pFFMpegData->pFormatCtx->streams[pFFMpegData->audioStreamNumber]->codec;

		// Find the decoder for the audio stream
		pFFMpegData->pAudioCodec=avcodec_find_decoder(pFFMpegData->pAudioCodecCtx->codec_id);
		if(pFFMpegData->pAudioCodec==NULL) {
			OutputDebugStringA("Unsupported codec!\n");
			return E_FAIL; // Codec not found
		}

		// Inform the codec that we can handle truncated bitstreams -- i.e.,
		// bitstreams where frame boundaries can fall in the middle of packets
		if(pFFMpegData->pAudioCodec->capabilities & CODEC_CAP_TRUNCATED)
			pFFMpegData->pAudioCodecCtx->flags|=CODEC_FLAG_TRUNCATED;

		// Open codec
		if(avcodec_open(pFFMpegData->pAudioCodecCtx, pFFMpegData->pAudioCodec)<0)
		{
			OutputDebugStringA("Open codec!\n");
			return E_FAIL; // Could not open codec
		}
		//Initialise Audio
		if(FAILED(InitAudio(pFFMpegData))){
			OutputDebugStringA("Can't init Audio ...\r\n");
			return E_FAIL;
		}
	}

	//-------------------------------------------------------------------------------------
	// 	Read video codec
	//-------------------------------------------------------------------------------------	
	if(pFFMpegData->videoStreamNumber>=0)//always true if we are here
	{
		// Get a pointer to the codec context for the video stream
		pFFMpegData->pVideoCodecCtx=pFFMpegData->pFormatCtx->streams[pFFMpegData->videoStreamNumber]->codec;

		// Find the decoder for the video stream
		pFFMpegData->pVideoCodec=avcodec_find_decoder(pFFMpegData->pVideoCodecCtx->codec_id);
		if(pFFMpegData->pVideoCodec==NULL) {
			OutputDebugStringA("Unsupported codec!\n");
			return E_FAIL; // Codec not found
		}

		// Inform the codec that we can handle truncated bitstreams -- i.e.,
		// bitstreams where frame boundaries can fall in the middle of packets
		if(pFFMpegData->pVideoCodec->capabilities & CODEC_CAP_TRUNCATED)
			pFFMpegData->pVideoCodecCtx->flags|=CODEC_FLAG_TRUNCATED;

		// Open codec
		if(avcodec_open(pFFMpegData->pVideoCodecCtx, pFFMpegData->pVideoCodec)<0)
		{
			OutputDebugStringA("Open codec!\n");
			return E_FAIL; // Could not open codec
		}

	}

	//-------------------------------------------------------------------------------------
	// 	Allocation of per frame buffer ..
	//-------------------------------------------------------------------------------------	
	// Allocate video frame
	pFFMpegData->pFrame=avcodec_alloc_frame();
	if(pFFMpegData->pFrame == NULL){
		OutputDebugStringA("Can't alloc video frame ...!\n");
		return E_FAIL; // Could not open codec
	}

	//Initialise yuv surface
//	InitYuvSurface(g_FFMpegData.pVideoCodecCtx->width, g_FFMpegData.pVideoCodecCtx->height);

	//Create Scale contexte
	pFFMpegData->pSWSContext = sws_getContext(
		pFFMpegData->pVideoCodecCtx->width, 
		pFFMpegData->pVideoCodecCtx->height, 
		pFFMpegData->pVideoCodecCtx->pix_fmt, 
		pFFMpegData->pVideoCodecCtx->width, 
		pFFMpegData->pVideoCodecCtx->height, 
		PIX_FMT_YUV420P, SWS_BILINEAR, 0, 0, 0
	);

	if(pFFMpegData->pSWSContext == NULL){
		OutputDebugStringA("Can't create pSWSContext ...!\n");
		return E_FAIL; // Could not open codec
	}

	//Create Audio buffer (freeed by callback)
	AudioBytes = (BYTE *)malloc(AVCODEC_MAX_AUDIO_FRAME_SIZE*sizeof(BYTE));//new BYTE[AVCODEC_MAX_AUDIO_FRAME_SIZE];
	if(AudioBytes == NULL){
		OutputDebugStringA("Can't alloc buffer for audio ...!\n");
		return E_FAIL; // Could not open codec
	}

	printf_s("%s\n",pFFMpegData->pFormatCtx->filename);

	InitThread(pFFMpegData);

	int done = 1;
	while(done){
		//
		WaitForSingleObject(pFFMpegData->hVideoMutex,INFINITE);


		FFMpegVideoFrame * videoFrame= GetVideoFrame(pFFMpegData);
		
		//FFMpegVideoFrame * videoFrame = NULL;
		if(videoFrame)
		{
			g_pFrameY = videoFrame->pYuv.pFrameY; 
			g_pFrameU = videoFrame->pYuv.pFrameU;
			g_pFrameV = videoFrame->pYuv.pFrameV;

			// Update the world
			Update();   
			// Render the scene
			Render();

			//Realease all video frame data ...
			SAFE_RELEASE(videoFrame->pYuv.pFrameY);
			SAFE_RELEASE(videoFrame->pYuv.pFrameU);
			SAFE_RELEASE(videoFrame->pYuv.pFrameV);
		}
		ReleaseMutex(pFFMpegData->hVideoMutex);
	}
	

	//-------------------------------------------------------------------------------------
	// 	Free buffer ...
	//-------------------------------------------------------------------------------------	
	// Free the YUV frame
	if(pFFMpegData->pFrame)
		av_free(pFFMpegData->pFrame);

	// Close the codec
	avcodec_close(pFFMpegData->pVideoCodecCtx);

	// Close the video file
	av_close_input_file(pFFMpegData->pFormatCtx);

	return S_OK;
}

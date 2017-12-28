#include "player.h"
#include "display.h"
#include "audio.h"

HRESULT video_ffmepg(char *);

extern	IDirect3DTexture9 * g_pFrameU;
extern	IDirect3DTexture9 * g_pFrameV;
extern	IDirect3DTexture9 * g_pFrameY;

extern "C"{
	int ptw32_processInitialize (void);
}
//-------------------------------------------------------------------------------------
// Name: main()
// Desc: The application's entry point
//-------------------------------------------------------------------------------------
void __cdecl main()
{
	ptw32_processInitialize();

	// Initialize Direct3D
	if( FAILED( InitD3D() ) )
		return;

	// Initialize the vertex buffer
	if( FAILED( InitScene() ) )
		return;

	// video_ffmepg("game:\\diehard3.avi");
	 video_ffmepg("game:\\intheend.mpeg");
	// video_ffmepg("game:\\video.avi");
	// video_ffmepg("game:\\onepiece.mp4");
	// video_ffmepg("game:\\f4.mp4");
	// video_ffmepg("game:\\I Am Legend - Trailer.mp4");
	// video_ffmepg("game:\\The Simpsons Movie - 1080p Trailer.mp4");
	// video_ffmepg("game:\\Tr2.mp4");
	// video_ffmepg("game:\\slim-tronlegacy.avi");
	// video_ffmepg("game:\\test.wmv");
}




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

	pFFMpegData->videoStreamNumber=-1;
	pFFMpegData->audioStreamNumber=-1;

	//-------------------------------------------------------------------------------------
	// Find stream
	//-------------------------------------------------------------------------------------	
	for(i=0; i<pFFMpegData->pFormatCtx->nb_streams; i++)
	{
		if(pFFMpegData->pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) 
		{
			pFFMpegData->videoStreamNumber=i;
		}
		else if(pFFMpegData->pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO) 
		{
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
		if(pFFMpegData->pAudioCodec==NULL) 
		{
			OutputDebugStringA("Unsupported Audio codec!\n");
			//return E_FAIL; // Codec not found
		}
		else
		{
			// Open codec
			if(avcodec_open(pFFMpegData->pAudioCodecCtx, pFFMpegData->pAudioCodec)<0)
			{
				OutputDebugStringA("Open Audio codec!\n");
				return E_FAIL; // Could not open codec
			}
			//Initialise Audio
			if(FAILED(InitAudio(pFFMpegData))){
				OutputDebugStringA("Can't init Audio ...\r\n");
				return E_FAIL;
			}
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
			OutputDebugStringA("Unsupported video codec!\n");
			return E_FAIL; // Codec not found
		}

		if(pFFMpegData->pVideoCodec->capabilities & CODEC_CAP_FRAME_THREADS ){
			pFFMpegData->pVideoCodecCtx->thread_type = FF_THREAD_FRAME;
			pFFMpegData->pVideoCodecCtx->thread_count = THREAD_COUNT;
		}

		// bitstreams where frame boundaries can fall in the middle of packets
		//if(pFFMpegData->pVideoCodec->capabilities & CODEC_CAP_TRUNCATED)
		//	pFFMpegData->pVideoCodecCtx->flags|=CODEC_FLAG_TRUNCATED;

		// Open codec
		if(avcodec_open(pFFMpegData->pVideoCodecCtx, pFFMpegData->pVideoCodec)<0)
		{
			OutputDebugStringA("Open codec!\n");
			return E_FAIL; // Could not open codec
		}

		pFFMpegData->fTimeBase = av_q2d(pFFMpegData->pFormatCtx->streams[pFFMpegData->videoStreamNumber]->time_base) * 1000;
		
	}

	//-------------------------------------------------------------------------------------
	// 	Allocation of per frame buffer ..
	//-------------------------------------------------------------------------------------	
	// Allocate video frame
	pFFMpegData->pFrame=avcodec_alloc_frame();
	if(pFFMpegData->pFrame == NULL)
	{
		OutputDebugStringA("Can't alloc video frame ...!\n");
		return E_FAIL; // Could not open codec
	}

	//Create Scale contexte
	pFFMpegData->pSWSContext = sws_getContext(
		pFFMpegData->pVideoCodecCtx->width, 
		pFFMpegData->pVideoCodecCtx->height, 
		pFFMpegData->pVideoCodecCtx->pix_fmt, 
		pFFMpegData->pVideoCodecCtx->width, 
		pFFMpegData->pVideoCodecCtx->height, 
		PIX_FMT_YUV420P, SWS_POINT, 0, 0, 0
	);

	if(pFFMpegData->pSWSContext == NULL)
	{
		OutputDebugStringA("Can't create pSWSContext ...!\n");
		return E_FAIL; // Could not open codec
	}

	InitThread(pFFMpegData);

	int done = 1;

	SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_NORMAL);

	while(done)
	{
		//Video
		WaitForSingleObject(pFFMpegData->hVideoMutex,INFINITE);
		FFMpegVideoFrame * videoFrame= GetVideoFrame(pFFMpegData);
		if(videoFrame)
		{
			//WaitForSingleObject(pFFMpegData->hVideoMutex,INFINITE);

			g_pFrameY = videoFrame->pYuv.pFrameY; 
			g_pFrameU = videoFrame->pYuv.pFrameU;
			g_pFrameV = videoFrame->pYuv.pFrameV;

			// Update the world
			// Update();   
			// Render the scene
			Render();

			//Release all video frame data ...
			SAFE_RELEASE(videoFrame->pYuv.pFrameY);
			free(videoFrame->pYuv.pFrameY);
			SAFE_RELEASE(videoFrame->pYuv.pFrameU);
			free(videoFrame->pYuv.pFrameU);
			SAFE_RELEASE(videoFrame->pYuv.pFrameV);
			free(videoFrame->pYuv.pFrameV);
		}
		ReleaseMutex(pFFMpegData->hVideoMutex);
	}
	
	// Todo: Close the threads ...


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

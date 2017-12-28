#pragma once

#include "stdafx.h"
#define UINT64_C(val) val##ui64
extern "C"{
#include <avcodec.h>
#include <avformat.h>
#include <swscale.h>
}

//Macro
#define SAFE_DELETE(a) if(a!=NULL){ delete (a); (a) = NULL; }
#define SAFE_FREE(a) if(a!=NULL){ free(a); (a) = NULL; }
#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

//Thread configuration
#define THREAD_COUNT 5
//#define THREAD_COUNT 8	
//#define AUDIO_THREAD 0 not used anymore
#define DECODE_THREAD 1 //Audio and decore are on the same thread 
//#define DECODE_THREAD 1 //Audio and decore are on the same thread 
//Buffer configuration
#define FFMPEG_VIDEO_BUFFER 50
#define FFMPEG_AUDIO_BUFFER 300

//Struct
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

typedef struct FFMpegAudioFrame {
	int64_t timestamp;
	BYTE * pBSoundBuffer;
	int size;
	int filled;
} FFMpegAudioFrame ;

typedef struct FFMpegPacketQueue {
  AVPacketList *first_pkt, *last_pkt;
  int nb_packets;
  int size;
  HANDLE hMutex;
} FFMpegPacketQueue;

typedef struct FFMpegData {
	//Fomat
	AVFormatContext *pFormatCtx;

	//Video
	AVCodecContext  *pVideoCodecCtx;
	AVCodec         *pVideoCodec;

	//Audio
	AVCodecContext  *pAudioCodecCtx;
	AVCodec			*pAudioCodec;
	int audioTime;

	//Scale
	SwsContext * pSWSContext;

	//Decoding stuff ...
	AVFrame         *pFrame; 
	AVPacket        packet;
	FFMpegPacketQueue packetQueue;

	//Stream ..
	int videoStreamNumber;
	int audioStreamNumber;

	//Mutex
	HANDLE hDecodeSemaphore;
	HANDLE hVideoMutex;
	HANDLE hAudioMutex;

	//Buffer
	FFMpegVideoFrame videoBuffer[FFMPEG_VIDEO_BUFFER];
	FFMpegAudioFrame audioBuffer[FFMPEG_AUDIO_BUFFER];

	//Time
	float fTimeBase;
	float fFrameRate;
	int64_t lastTimeStamp;
	int64_t startTime;

} FFMpegData, *PFFMpegData;

//Proto
DWORD WINAPI FfmpegDecodeThread( LPVOID lpParam );
DWORD WINAPI SoundOutputThread( LPVOID lpParam );

HRESULT InitThread(FFMpegData * data);

FFMpegVideoFrame * GetVideoFrame(FFMpegData * data);
#include "player.h"

DWORD dwDecodeThreadRunning = 1;
HRESULT AddSoundToQueue(FFMpegData * data);
HRESULT AddFrameToQueue(
	FFMpegData * data
);

HRESULT InitThread(FFMpegData * data){

	data->hDecodeSemaphore = CreateSemaphore( 
        NULL,           // default security attributes
        1,				// initial count
        10,				// maximum count
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

	data->startTime = GetTickCount();

	DWORD dwThreadId = 0;
	HANDLE hThread = CreateThread(NULL, 0, FfmpegDecodeThread, data, CREATE_SUSPENDED, &dwThreadId);
	XSetThreadProcessor(hThread,DECODE_THREAD);
	ResumeThread(hThread);

/*
	DWORD dwAudioThreadId = 0;
	HANDLE hAudioThread = CreateThread(NULL, 0, SoundOutputThread, data, CREATE_SUSPENDED, &dwThreadId);
	XSetThreadProcessor(hAudioThread,AUDIO_THREAD);
	ResumeThread(hAudioThread);
*/	
	if(hThread)
		return S_OK;
	else
		return E_FAIL;
};




DWORD WINAPI FfmpegDecodeThread( LPVOID lpParam ){
	if(lpParam==NULL)
		DebugBreak();

	FFMpegData * FFData = (FFMpegData *)lpParam;

	int frameFinished = 0;
	int len = 0;

	while(dwDecodeThreadRunning){
		//-------------------------------------------------------------------------------------
		// 	Main loop
		//-------------------------------------------------------------------------------------	

		//On as le droit ?
		WaitForSingleObject(FFData->hDecodeSemaphore,INFINITE);
		int decode = av_read_frame(FFData->pFormatCtx, &FFData->packet);
		//Release the semaphore ...
		ReleaseSemaphore(FFData->hDecodeSemaphore, 1, NULL);

		/* if we did not get a packet, we wait a bit and try again */
        if(decode < 0) {
            /* thread is idle */
			// Sleep(10);
            // continue;
			//End of stream ?
			av_free_packet(&FFData->packet);
			return E_FAIL;
        }
		
		//we got a packet !!
		if(1) {
			//-------------------------------------------------------------------------------------
			// 	Handle video stream
			//-------------------------------------------------------------------------------------
			if(FFData->videoStreamNumber>=0)
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
			if(FFData->audioStreamNumber>=0)
			if(FFData->packet.stream_index==FFData->audioStreamNumber){
				//Not queued ... 
				//AddSoundToQueue(FFData);
			}
		}
		// Free the packet that was allocated by av_read_frame
		av_free_packet(&FFData->packet);
	}

	return S_OK;

};


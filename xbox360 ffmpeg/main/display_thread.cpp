#include "player.h"
#include "display.h"

unsigned int getPosition(FFMpegData * data){
	return GetTickCount() - data->startTime;
};

//Get a video frame Need Lock Arround
FFMpegVideoFrame * GetVideoFrame(FFMpegData * data){
	FFMpegVideoFrame * pFrame = NULL;//Error ...

	int i=0;
	while(i<FFMPEG_VIDEO_BUFFER){
		if(data->videoBuffer[i].filled==1){
			DWORD Pos = getPosition(data);
			if(data->videoBuffer[i].timestamp <= Pos)//Display bug happen because of this ....
			{
				data->videoBuffer[i].filled=0;
				return &data->videoBuffer[i];
			}
		}
		i++;

		if(i==FFMPEG_VIDEO_BUFFER){
			return NULL;
		}
	}
	return NULL;
};


HRESULT AddFrameToQueue(
	FFMpegData * data
){
	//Find a place ...
	int i=0;
	
	while(1){
		WaitForSingleObject(data->hVideoMutex,INFINITE);
		if(data->videoBuffer[i].filled==0){
			break;
		}
		ReleaseMutex(data->hVideoMutex);
		i++;

		if(i==FFMPEG_VIDEO_BUFFER){
			i=0;
			Sleep(40);//Wait 40 msec... restart...
		}
	}

	data->videoBuffer[i].filled = 1;
	//if(data->packet.dts == AV_NOPTS_VALUE){
	if(data->packet.dts == 0x8000000000000000){
		//can't calculate
		data->videoBuffer[i].timestamp = data->lastTimeStamp + data->fTimeBase;
	}
	else
	{
		//get it from packet
		data->videoBuffer[i].timestamp = data->fTimeBase * data->packet.dts;
	}

	data->lastTimeStamp = data->videoBuffer[i].timestamp;

	AVPicture pict;
	memset(&pict, 0, sizeof(pict));

	InitYuvSurface(
		&data->videoBuffer[i].pYuv.pFrameY,
		&data->videoBuffer[i].pYuv.pFrameU,
		&data->videoBuffer[i].pYuv.pFrameV,
		data->pVideoCodecCtx->width,
		data->pVideoCodecCtx->height
	);


	if(data->videoBuffer[i].pYuv.pFrameY==NULL){
		DebugBreak();
	}
	if(data->videoBuffer[i].pYuv.pFrameU==NULL){
		DebugBreak();
	}
	if(data->videoBuffer[i].pYuv.pFrameV==NULL){
		DebugBreak();
	}

	//Lock texture to access data
	D3DLOCKED_RECT lockRectY;
	D3DLOCKED_RECT lockRectU;
	D3DLOCKED_RECT lockRectV;

	data->videoBuffer[i].pYuv.pFrameY->LockRect( 0, &lockRectY, NULL, 0 );
	data->videoBuffer[i].pYuv.pFrameU->LockRect( 0, &lockRectU, NULL, 0 );
	data->videoBuffer[i].pYuv.pFrameV->LockRect( 0, &lockRectV, NULL, 0 );

	pict.data[0] = (uint8_t*)lockRectY.pBits;
	pict.data[1] = (uint8_t*)lockRectU.pBits;
	pict.data[2] = (uint8_t*)lockRectV.pBits;

	pict.linesize[0] = lockRectY.Pitch;
	pict.linesize[1] = lockRectU.Pitch;
	pict.linesize[2] = lockRectV.Pitch;

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
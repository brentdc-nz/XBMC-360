#pragma once

#include "DVDVideoCodec.h"
#include "..\..\VideoRenderers\RenderManager.h" // for RGBImage definition

//#include "../../VideoRenderers/XBoxRenderer.h" // for YV12Image definition

class CDVDCodecUtils
{
public:
	//static DVDVideoPicture* AllocatePicture(int iWidth, int iHeight);
	static void FreePicture(DVDVideoPicture* pPicture);
	//static bool CopyPicture(DVDVideoPicture* pDst, DVDVideoPicture* pSrc);
	static bool CopyPictureToOverlay(RGB32Image_t* pImage, DVDVideoPicture *pSrc);
};
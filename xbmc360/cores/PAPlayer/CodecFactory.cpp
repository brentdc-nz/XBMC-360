#include "CodecFactory.h"
#include "FLACcodec.h"
#include "MP3codec.h"

ICodec* CodecFactory::CreateCodec(const CStdString& strFileType)
{
	// TODO: More to be added !

	if (strFileType.Equals("mp3") || strFileType.Equals("mp2"))
		return new MP3Codec();

	if (strFileType.Equals("flac"))
		return new FLACCodec();

	return NULL;
}

ICodec* CodecFactory::CreateCodecDemux(const CStdString& strFile, const CStdString& strContent, unsigned int filecache)
{
	CURL urlFile(strFile);

	// TODO !

	// Default
	return CreateCodec(urlFile.GetFileType());
}
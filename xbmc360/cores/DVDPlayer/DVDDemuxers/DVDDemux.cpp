#include "DVDDemux.h"

void CDemuxStream::GetStreamName( std::string& strInfo )
{   
	if(language[0] == 0)
		strInfo = "";
	else
	{
		CStdString name;
//		g_LangCodeExpander.Lookup( name, language );
		strInfo = name;
	}
}

void CDemuxStream::SetDiscard(AVDiscard discard)
{
	return;
}

void CDemuxStreamAudio::GetStreamType(std::string& strInfo)
{
	char sInfo[64];
  
	if(codec == CODEC_ID_AC3) strcpy(sInfo, "AC3 ");
	else if(codec == CODEC_ID_DTS) strcpy(sInfo, "DTS ");
	else if(codec == CODEC_ID_MP2) strcpy(sInfo, "MP2 ");
	else strcpy(sInfo, "");
    
	if(iChannels == 1) strcat(sInfo, "Mono");
	else if(iChannels == 2) strcat(sInfo, "Stereo");
	else if(iChannels == 6) strcat(sInfo, "5.1");
	else if(iChannels != 0)
	{
		char temp[32];
		sprintf(temp, " %d %s", iChannels, "Channels");
		strcat(sInfo, temp);
	}
	strInfo = sInfo;
}
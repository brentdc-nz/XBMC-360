//
// TODO: text-subtitle stack not ported (DVDSubtitles/ parsers, DVDOverlayCodec*),
// those paths are commented below - only the DVD SPU path (menus/subtitles)
// and the CLUT update are active.

#include "utils\Log.h"
#include "DVDPlayerSubtitle.h"
#include "DVDCodecs\Overlay\DVDOverlay.h"
#include "DVDCodecs\Overlay\DVDOverlaySpu.h"
#include "DVDCodecs\Overlay\DVDOverlayText.h"
#include "DVDClock.h"
#include "DVDInputStreams\DVDInputStream.h"

using namespace std;

CDVDPlayerSubtitle::CDVDPlayerSubtitle(CDVDOverlayContainer* pOverlayContainer)
{
	m_pOverlayContainer = pOverlayContainer;

	m_lastPts = DVD_NOPTS_VALUE;
}

CDVDPlayerSubtitle::~CDVDPlayerSubtitle()
{
	CloseStream(false);
}


void CDVDPlayerSubtitle::Flush()
{
	SendMessage(new CDVDMsg(CDVDMsg::GENERAL_FLUSH));
}

void CDVDPlayerSubtitle::SendMessage(CDVDMsg* pMsg)
{
	if (pMsg->IsType(CDVDMsg::DEMUXER_PACKET))
	{
		CDVDMsgDemuxerPacket* pMsgDemuxerPacket = (CDVDMsgDemuxerPacket*)pMsg;
		DemuxPacket* pPacket = pMsgDemuxerPacket->GetPacket();

		// TODO: overlay codecs (text subtitles) not ported - xbmc4xbox:
		//  if (m_pOverlayCodec) { ... m_pOverlayCodec->Decode(pPacket) ... }
		if (m_streaminfo.codec == AV_CODEC_ID_DVD_SUBTITLE)
		{
			CDVDOverlaySpu* pSPUInfo = m_dvdspus.AddData(pPacket->pData, pPacket->iSize, pPacket->pts);
			if (pSPUInfo)
			{
				CLog::Log(LOGDEBUG, "CDVDPlayer::ProcessSubData: Got complete SPU packet");
				m_pOverlayContainer->Add(pSPUInfo);
				pSPUInfo->Release();
			}
		}

	}
	else if( pMsg->IsType(CDVDMsg::SUBTITLE_CLUTCHANGE) )
	{
		CDVDMsgSubtitleClutChange* pData = (CDVDMsgSubtitleClutChange*)pMsg;
		for (int i = 0; i < 16; i++)
		{
			BYTE* color = m_dvdspus.m_clut[i];
			BYTE* t = (BYTE*)pData->m_data[i];

			color[0] = t[2]; // Y
			color[1] = t[0]; // Cr
			color[2] = t[1]; // Cb
		}
		m_dvdspus.m_bHasClut = true;
	}
	else if( pMsg->IsType(CDVDMsg::GENERAL_FLUSH)
				|| pMsg->IsType(CDVDMsg::GENERAL_RESET) )
	{
		m_dvdspus.Reset();
		// TODO: not ported - xbmc4xbox: if (m_pSubtitleFileParser) m_pSubtitleFileParser->Reset();
		// TODO: not ported - xbmc4xbox: if (m_pOverlayCodec) m_pOverlayCodec->Flush();

		/* We must flush active overlays on flush or if we have a file
		 * parser since it will re-populate active items.  */
		if(pMsg->IsType(CDVDMsg::GENERAL_FLUSH)) // TODO: not ported - xbmc4xbox: || m_pSubtitleFileParser)
			m_pOverlayContainer->Clear();

		m_lastPts = DVD_NOPTS_VALUE;
	}

	pMsg->Release();
}

bool CDVDPlayerSubtitle::OpenStream(CDVDStreamInfo &hints, string &filename)
{
	CloseStream(false);
	m_streaminfo = hints;

	// okey check if this is a filesubtitle
	if(filename.size() && filename != "dvd" )
	{
		// TODO: subtitle file parsers (srt/sub/ssa...) not ported - xbmc4xbox:
		//  m_pSubtitleFileParser = CDVDFactorySubtitle::CreateParser(filename);
		CLog::Log(LOGERROR, "%s - External subtitle files not ported [%s]", __FUNCTION__, filename.c_str());
		CloseStream(false);
		return false;
	}

	// dvd's use special subtitle decoder
	if(hints.codec == AV_CODEC_ID_DVD_SUBTITLE && filename == "dvd")
	{
		return true;
	}

	// TODO: overlay codecs (text subtitles in containers) not ported - xbmc4xbox:
	//  m_pOverlayCodec = CDVDFactoryCodec::CreateOverlayCodec(hints);
	//  if(m_pOverlayCodec) return true;
	CLog::Log(LOGERROR, "%s - Unable to init overlay codec (text subtitles not ported)", __FUNCTION__);
	return false;
}

void CDVDPlayerSubtitle::CloseStream(bool flush)
{
	// TODO: not ported - xbmc4xbox: SAFE_DELETE(m_pSubtitleStream); SAFE_DELETE(m_pSubtitleFileParser); SAFE_DELETE(m_pOverlayCodec);

	m_dvdspus.FlushCurrentPacket();

	if(flush)
		m_pOverlayContainer->Clear();
}

void CDVDPlayerSubtitle::Process(double pts)
{
	// TODO: subtitle file parser processing not ported - xbmc4xbox:
	//  if (m_pSubtitleFileParser) { ... m_pSubtitleFileParser->Parse(pts) ... }
}

bool CDVDPlayerSubtitle::AcceptsData()
{
	// FIXME : This may still be causing problems + magic number :(
	return m_pOverlayContainer->GetSize() < 5;
}

void CDVDPlayerSubtitle::GetCurrentSubtitle(CStdString& strSubtitle, double pts)
{
	strSubtitle = "";

	Process(pts); // TODO: move to separate thread?

	m_pOverlayContainer->Lock();
	VecOverlays* pOverlays = m_pOverlayContainer->GetOverlays();
	if (pOverlays)
	{
		for(vector<CDVDOverlay*>::iterator it = pOverlays->begin();it != pOverlays->end();it++)
		{
			CDVDOverlay* pOverlay = *it;

			if (pOverlay->IsOverlayType(DVDOVERLAY_TYPE_TEXT) 
			&& (pOverlay->iPTSStartTime <= pts)
			&& (pOverlay->iPTSStopTime >= pts || pOverlay->iPTSStopTime == 0LL))
			{
				CDVDOverlayText::CElement* e = ((CDVDOverlayText*)pOverlay)->m_pHead;
				while (e)
				{
					if (e->IsElementType(CDVDOverlayText::ELEMENT_TYPE_TEXT))
					{
						CDVDOverlayText::CElementText* t = (CDVDOverlayText::CElementText*)e;
						strSubtitle += t->m_text;
						strSubtitle += "\n";
					}
					e = e->pNext;
				}
			}
		}
	}
	m_pOverlayContainer->Unlock();
	strSubtitle.TrimRight('\n');
}

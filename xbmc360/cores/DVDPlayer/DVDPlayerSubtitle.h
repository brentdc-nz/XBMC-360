#pragma once

// TODO: text-subtitle stack not ported (DVDSubtitles/, DVDOverlayCodec*) -
// xbmc4xbox additionally had m_pSubtitleStream/m_pSubtitleFileParser/m_pOverlayCodec
// and included DVDSubtitles/DVDFactorySubtitle.h

#include "DVDOverlayContainer.h"
#include "DVDStreamInfo.h"
#include "DVDMessageQueue.h"
#include "DVDDemuxSPU.h"

class CDVDInputStream;
class CDVDInputStreamNavigator;

class CDVDPlayerSubtitle
{
public:
	CDVDPlayerSubtitle(CDVDOverlayContainer* pOverlayContainer);
	~CDVDPlayerSubtitle();

	void Process(double pts);
	void Flush();
	void FindSubtitles(const char* strFilename);
	void GetCurrentSubtitle(CStdString& strSubtitle, double pts);
	int GetSubtitleCount();

	void UpdateOverlayInfo(CDVDInputStreamNavigator* pStream, int iAction) { m_pOverlayContainer->UpdateOverlayInfo(pStream, &m_dvdspus, iAction); }

	bool AcceptsData();
	void SendMessage(CDVDMsg* pMsg);
	bool OpenStream(CDVDStreamInfo &hints, std::string& filename);
	void CloseStream(bool flush);

	bool IsStalled() { return m_pOverlayContainer->GetSize() == 0; }
private:
	CDVDOverlayContainer* m_pOverlayContainer;  

	// TODO: not ported - xbmc4xbox:
	//  CDVDSubtitleStream*  m_pSubtitleStream;
	//  CDVDSubtitleParser*  m_pSubtitleFileParser;
	//  CDVDOverlayCodec*    m_pOverlayCodec;
	CDVDDemuxSPU        m_dvdspus;

	CDVDStreamInfo      m_streaminfo;
	double              m_lastPts;
};

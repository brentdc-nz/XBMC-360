#include "DVDPlayer.h"
#include "utils\Log.h"
#include "utils\MathUtils.h"
#include "Application.h"
#include "Settings.h"
#include "DVDInputStreams\DVDFactoryInputStream.h"
#include "DVDDemuxers\DVDFactoryDemuxer.h"
#include "DVDDemuxers\DVDDemuxUtils.h"
#include "DVDPerformanceCounter.h"
#include "AdvancedSettings.h"

using namespace std;

void CSelectionStreams::Clear(StreamType type, StreamSource source)
{
	CSingleLock lock(m_section);

	for(int i = m_Streams.size()-1; i >= 0; i--)
	{
		if(type && m_Streams[i].type != type)
			continue;

		if(source && m_Streams[i].source != source)
			continue;

		m_Streams.erase(m_Streams.begin() + i);
	}
}

SelectionStream& CSelectionStreams::Get(StreamType type, int index)
{
	CSingleLock lock(m_section);
	int count = -1;

	for(int i = 0; i < (int)m_Streams.size(); i++)
	{
		if(m_Streams[i].type != type)
			continue;

		count++;

		if(count == index)
			return m_Streams[i];
	}
	CLog::Log(LOGERROR, "%s - failed to get stream", __FUNCTION__);
	return m_invalid;
}

bool CSelectionStreams::Get(StreamType type, CDemuxStream::EFlags flag, SelectionStream& out)
{
	CSingleLock lock(m_section);

	for(int i = 0; i < (int)m_Streams.size(); i++)
	{
		if(m_Streams[i].type != type)
			continue;
		
		if((m_Streams[i].flags & flag) != flag)
			continue;

		out = m_Streams[i];
		return true;
	}
	return false;
}

int CSelectionStreams::IndexOf(StreamType type, int source, int id)	
{
	CSingleLock lock(m_section);
	int count = -1;

	for(int i = 0; i < (int)m_Streams.size(); i++)
	{
		if(type && m_Streams[i].type != type)
			continue;
		
		count++;
		
		if(source && m_Streams[i].source != source)
			continue;

		if(id < 0)
			continue;
		
		if(m_Streams[i].id == id)
			return count;
	}

	if(id < 0)
		return count;
	else
		return -1;
}

int CSelectionStreams::Source(StreamSource source, std::string filename)
{
	CSingleLock lock(m_section);
	int index = source - 1;

	for(int i = 0; i < (int)m_Streams.size(); i++)
	{
		SelectionStream &s = m_Streams[i];
		if(STREAM_SOURCE_MASK(s.source) != source)
			continue;

		// If it already exists, return same
		if(s.filename == filename)
			return s.source;

		if(index < s.source)
			index = s.source;
	}

	// Return next index
	return index + 1;
}

void CSelectionStreams::Update(SelectionStream& s)
{
	CSingleLock lock(m_section);
	int index = IndexOf(s.type, s.source, s.id);

	if(index >= 0)
		Get(s.type, index) = s;
	else
		m_Streams.push_back(s);
}

void CSelectionStreams::Update(CDVDInputStream* input, CDVDDemux* demuxer)
{
	if(input && input->IsStreamType(DVDSTREAM_TYPE_DVD))
	{
/*		CDVDInputStreamNavigator* nav = (CDVDInputStreamNavigator*)input;
		string filename = nav->GetFileName();
		int source = Source(STREAM_SOURCE_NAV, filename);

		int count;
		count = nav->GetAudioStreamCount();
		
		for(int i = 0; i < count; i++)
		{
			SelectionStream s;
			s.source   = source;
			s.type     = STREAM_AUDIO;
			s.id       = i;
			s.name     = nav->GetAudioStreamLanguage(i);
			s.flags    = CDemuxStream::FLAG_NONE;
			s.filename = filename;
			Update(s);
		}

		count = nav->GetSubTitleStreamCount();
		for(int i = 0; i < count; i++)
		{
			SelectionStream s;
			s.source   = source;
			s.type     = STREAM_SUBTITLE;
			s.id       = i;
			s.name     = nav->GetSubtitleStreamLanguage(i);
			s.flags    = CDemuxStream::FLAG_NONE;
			s.filename = filename;
			Update(s);
		}
*/	}
	else if(demuxer)
	{
		string filename = demuxer->GetFileName();
		int count = demuxer->GetNrOfStreams();
		int source;

		if(input) // Hack to know this is sub decoder
			source = Source(STREAM_SOURCE_DEMUX, filename);
		else
			source = Source(STREAM_SOURCE_DEMUX_SUB, filename);

		for(int i = 0; i < count; i++)
		{
			CDemuxStream* stream = demuxer->GetStream(i);
			// Make sure stream is marked with right source
			stream->source = source;

			SelectionStream s;
			s.source   = source;
			s.type     = stream->type;
			s.id       = stream->iId;
			s.language = stream->language;
			s.flags    = stream->flags;
			s.filename = demuxer->GetFileName();
			stream->GetStreamName(s.name);

			if(stream->type == STREAM_AUDIO)
			{
				std::string type;
				((CDemuxStreamAudio*)stream)->GetStreamType(type);
				if(type.length() > 0)
				{
					if(s.name.length() > 0)
						s.name += " - ";
					
					s.name += type;
				}
			}
			Update(s);
		}
	}
}

CDVDPlayer::CDVDPlayer(IPlayerCallback& callback)
	: IPlayer(callback),
      CThread(),
      m_CurrentAudio(STREAM_AUDIO),
      m_CurrentVideo(STREAM_VIDEO),
      m_CurrentSubtitle(STREAM_SUBTITLE),
	  m_messenger("player"),
	  m_dvdPlayerVideo(&m_clock/*, &m_overlayContainer*/, m_messenger),
	  m_dvdPlayerAudio(&m_clock, m_messenger)
{
	m_pDemuxer = NULL;
	m_pSubtitleDemuxer = NULL;
	m_pInputStream = NULL;

	m_hReadyEvent = CreateEvent(NULL, true, false, NULL);

	InitializeCriticalSection(&m_critStreamSection);

	m_dvd.Clear();
	m_State.Clear();
	m_UpdateApplication = 0;

	m_bAbortRequest = false;
	m_errorCount = 0;
	m_playSpeed = DVD_PLAYSPEED_NORMAL;
	m_caching = CACHESTATE_DONE;

#ifdef DVDDEBUG_MESSAGE_TRACKER
	g_dvdMessageTracker.Init();
#endif
}

CDVDPlayer::~CDVDPlayer()
{
	CloseFile();

	CloseHandle(m_hReadyEvent);
	DeleteCriticalSection(&m_critStreamSection);
#ifdef DVDDEBUG_MESSAGE_TRACKER
	g_dvdMessageTracker.DeInit();
#endif
}

bool CDVDPlayer::OpenFile(const CFileItem& file, const CPlayerOptions &options)
{
	try
	{
		CLog::Log(LOGNOTICE, "DVDPlayer: Opening: %s", file.GetPath().c_str());

		// If playing a file close it first
		// this has to be changed so we won't have to close it.
		if(ThreadHandle())
			CloseFile();

		m_bAbortRequest = false;
		SetPlaySpeed(DVD_PLAYSPEED_NORMAL);

		m_State.Clear();
		m_UpdateApplication = 0;

		m_PlayerOptions = options;
		m_item     = file;
//		m_mimetype  = file.GetMimeType();
		m_filename = file.GetPath();

		ResetEvent(m_hReadyEvent);
		Create();
		WaitForSingleObject(m_hReadyEvent, INFINITE);

		// Playback might have been stopped due to some error
		if (m_bStop || m_bAbortRequest)
			return false;

		return true;
	}
	catch(...)
	{
		CLog::Log(LOGERROR, "%s - Exception thrown on open", __FUNCTION__);
		return false;
	}
}

bool CDVDPlayer::CloseFile()
{
	CLog::Log(LOGNOTICE, "CDVDPlayer::CloseFile()");

	// Unpause the player
	SetPlaySpeed(DVD_PLAYSPEED_NORMAL);

	// Set the abort request so that other threads can finish up
	m_bAbortRequest = true;

	// Tell demuxer to abort
	if(m_pDemuxer)
		m_pDemuxer->Abort();

#ifdef _ENABLE_SUBTITLES
	if(m_pSubtitleDemuxer)
		m_pSubtitleDemuxer->Abort();
#endif

	CLog::Log(LOGNOTICE, "DVDPlayer: waiting for threads to exit");

	// Wait for the main thread to finish up
	// since this main thread cleans up all other resources and threads
	// we are done after the StopThread call
	StopThread();

	m_Edl.Clear();
	m_EdlAutoSkipMarkers.Clear();

	CLog::Log(LOGNOTICE, "DVDPlayer: finished waiting");

	return true;
}

bool CDVDPlayer::IsPlaying() const
{
	return !m_bStop;
}

bool CDVDPlayer::IsPaused() const
{
	return (m_playSpeed == DVD_PLAYSPEED_PAUSE || m_caching == CACHESTATE_FULL);
}

bool CDVDPlayer::HasVideo() const
{
	if(m_pInputStream)
	{
		if(m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD) || m_CurrentVideo.id >= 0)
			return true;
	}
	return false;
}

bool CDVDPlayer::HasAudio() const
{
	return (m_CurrentAudio.id >= 0);
}

void CDVDPlayer::OnStartup()
{
	CThread::SetName("CDVDPlayer");
	m_CurrentVideo.Clear();
	m_CurrentAudio.Clear();
	m_CurrentSubtitle.Clear();

	m_messenger.Init();

	g_dvdPerformanceCounter.EnableMainPerformance(ThreadHandle());
}

void CDVDPlayer::Process()
{
	if(!OpenInputStream())
	{
		m_bAbortRequest = true;
		return;
	}

	if(m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD)) //TODO
	{
		CLog::Log(LOGNOTICE, "DVDPlayer: playing a dvd with menu's");
		m_PlayerOptions.starttime = 0;
/*
		if(m_PlayerOptions.state.size() > 0)
			((CDVDInputStreamNavigator*)m_pInputStream)->SetNavigatorState(m_PlayerOptions.state);
		else
			((CDVDInputStreamNavigator*)m_pInputStream)->EnableSubtitleStream(g_stSettings.m_currentVideoSettings.m_SubtitleOn);

		g_stSettings.m_currentVideoSettings.m_SubtitleCached = true;
*/	}

	if(!OpenDemuxStream())
	{
		m_bAbortRequest = true;
		return;
	}

	// Allow renderer to switch to fullscreen if requested
	m_dvdPlayerVideo.EnableFullscreen(m_PlayerOptions.fullscreen);

	OpenDefaultStreams();

	// Look for any EDL files
	m_Edl.Clear();
	m_EdlAutoSkipMarkers.Clear();
	float fFramesPerSecond;
	if(m_CurrentVideo.id >= 0 && m_CurrentVideo.hint.fpsrate > 0 && m_CurrentVideo.hint.fpsscale > 0)
	{
		fFramesPerSecond = (float)m_CurrentVideo.hint.fpsrate / (float)m_CurrentVideo.hint.fpsscale;
		m_Edl.ReadEditDecisionLists(m_filename, fFramesPerSecond, m_CurrentVideo.hint.height);
	}

	// Check to see if the demuxer should start at something other than time 0. This will be the case
	// if there was a start time specified as part of the "Start from where last stopped" (aka
	// auto-resume) feature or if there is an EDL cut or commercial break that starts at time 0.
	CEdl::Cut cut;
	int starttime = 0;
	if(m_PlayerOptions.starttime > 0)
	{
		starttime = m_Edl.RestoreCutTime((__int64)m_PlayerOptions.starttime * 1000); // s to ms
		CLog::Log(LOGDEBUG, "%s - Start position set to last stopped position: %d", __FUNCTION__, starttime);
	}
	else if(m_Edl.InCut(0, &cut) && (cut.action == CEdl::CUT || cut.action == CEdl::COMM_BREAK))
	{
		starttime = cut.end;
		CLog::Log(LOGDEBUG, "%s - Start position set to end of first cut or commercial break: %d", __FUNCTION__, starttime);

		if(cut.action == CEdl::COMM_BREAK)
		{
			// Setup auto skip markers as if the commercial break had been skipped using standard detection
			m_EdlAutoSkipMarkers.commbreak_start = cut.start;
			m_EdlAutoSkipMarkers.commbreak_end   = cut.end;
			m_EdlAutoSkipMarkers.seek_to_start   = true;
		}
	}

	if(starttime > 0)
	{
		double startpts = DVD_NOPTS_VALUE;
		if(m_pDemuxer)
		{
			if(m_pDemuxer->SeekTime(starttime, false, &startpts))
				CLog::Log(LOGDEBUG, "%s - starting demuxer from: %d", __FUNCTION__, starttime);
			else
				CLog::Log(LOGDEBUG, "%s - failed to start demuxing from: %d", __FUNCTION__, starttime);
		}

		if(m_pSubtitleDemuxer)
		{
			if(m_pSubtitleDemuxer->SeekTime(starttime, false, &startpts))
				CLog::Log(LOGDEBUG, "%s - starting subtitle demuxer from: %d", __FUNCTION__, starttime);
			else
				CLog::Log(LOGDEBUG, "%s - failed to start subtitle demuxing from: %d", __FUNCTION__, starttime);
		}
	}

	// Make sure application know our info
	UpdateApplication(0);
	UpdatePlayState(0);

	if(m_PlayerOptions.identify == false)
		m_callback.OnPlayBackStarted();

	// We are done initializing now, set the readyevent
	SetEvent(m_hReadyEvent);

	// Make sure all selected stream have data on startup
	// Use full caching for Xbox since it sometimes experiences weird A/V desync at start
#if 0//def _XBOX  
	SetCaching(CACHESTATE_FULL);
#else
	SetCaching(CACHESTATE_INIT);
#endif

	while(!m_bAbortRequest)
	{
		// Handle messages send to this thread, like seek or demuxer reset requests
		HandleMessages();

		if(m_bAbortRequest)
			break;

		// Should we open a new input stream?
		if(!m_pInputStream)
		{
			if(OpenInputStream() == false)
			{
				m_bAbortRequest = true;
				break;
			}
		}

		// Should we open a new demuxer?
		if(!m_pDemuxer)
		{
			if(m_pInputStream->NextStream() == false)
				break;

			if(m_pInputStream->IsEOF())
				break;

			if(OpenDemuxStream() == false)
			{
				m_bAbortRequest = true;
				break;
			}

			OpenDefaultStreams();
			UpdateApplication(0);
			UpdatePlayState(0);
		}

		// Handle eventual seeks due to playspeed
		HandlePlaySpeed();

		// Update player state
		UpdatePlayState(200);

		// Update application with our state
		UpdateApplication(1000);

		// If the queues are full, no need to read more
		if((!m_dvdPlayerAudio.AcceptsData() && m_CurrentAudio.id >= 0)
		||  (!m_dvdPlayerVideo.AcceptsData() && m_CurrentVideo.id >= 0))
		{
			Sleep(10);
			if(m_caching != CACHESTATE_INIT
			|| (m_dvdPlayerAudio.m_messageQueue.GetDataSize() == 0 && m_CurrentAudio.id >= 0)
			|| (m_dvdPlayerVideo.m_messageQueue.GetDataSize() == 0 && m_CurrentVideo.id >= 0))
			{
				SetCaching(CACHESTATE_DONE);
				SAFE_RELEASE(m_CurrentAudio.startsync);
				SAFE_RELEASE(m_CurrentVideo.startsync);
			}
			continue;
		}

		// Always yield to players if they have data
		if((m_dvdPlayerAudio.m_messageQueue.GetDataSize() > 0 || m_CurrentAudio.id < 0)
		&& (m_dvdPlayerVideo.m_messageQueue.GetDataSize() > 0 || m_CurrentVideo.id < 0))
			Sleep(0);

		DemuxPacket* pPacket = NULL;
		CDemuxStream *pStream = NULL;

		ReadPacket(pPacket, pStream);

		if(pPacket && !pStream)
		{
			// Probably a empty packet, just free it and move on
			CDVDDemuxUtils::FreeDemuxPacket(pPacket);
			continue;
		}

		if(!pPacket)
		{
			// When paused, demuxer could be be returning empty
			if(m_playSpeed == DVD_PLAYSPEED_PAUSE)
				continue;

			// If there is another stream available, let
			// player reopen demuxer
			if(m_pInputStream->NextStream())
			{
				SAFE_DELETE(m_pDemuxer);
				continue;
			}

			if(m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD)) //TODO
			{
/*				CDVDInputStreamNavigator* pStream = static_cast<CDVDInputStreamNavigator*>(m_pInputStream);

				// Stream is holding back data until demuxer has flushed
				if(pStream->IsHeld())
				{
					pStream->SkipHold();
					continue;
				}

				// Stills will be skipped
				if(m_dvd.state == DVDSTATE_STILL)
				{
					if(m_dvd.iDVDStillTime > 0)
					{
						if(GetTickCount() >= (m_dvd.iDVDStillStartTime + m_dvd.iDVDStillTime))
						{
							m_dvd.iDVDStillTime = 0;
							m_dvd.iDVDStillStartTime = 0;
							m_dvd.state = DVDSTATE_NORMAL;
							pStream->SkipStill();
							continue;
						}
					}
				}

				// If playing a main title DVD/ISO rip, there is no menu structure so
				// dvdnav will tell us it's done by setting EOF on the stream.
				if(pStream->IsEOF())
					break;
*/
				// Always continue on dvd's
				Sleep(100);
				continue;
			}

			// Make sure we tell all players to finish it's data
			if(m_CurrentAudio.inited)
				m_dvdPlayerAudio.SendMessage   (new CDVDMsg(CDVDMsg::GENERAL_EOF));
			
			if(m_CurrentVideo.inited)
				m_dvdPlayerVideo.SendMessage   (new CDVDMsg(CDVDMsg::GENERAL_EOF));

#ifdef _ENABLE_SUBTITLES
			if(m_CurrentSubtitle.inited)
				m_dvdPlayerSubtitle.SendMessage(new CDVDMsg(CDVDMsg::GENERAL_EOF));
#endif			
			m_CurrentAudio.inited    = false;
			m_CurrentVideo.inited    = false;
			m_CurrentSubtitle.inited = false;
			m_CurrentAudio.started    = false;
			m_CurrentVideo.started    = false;
			m_CurrentSubtitle.started = false;

			// If we are caching, start playing it again
			SetCaching(CACHESTATE_DONE);

			// While players are still playing, keep going to allow seekbacks
			if(m_dvdPlayerAudio.m_messageQueue.GetDataSize() > 0
			|| m_dvdPlayerVideo.m_messageQueue.GetDataSize() > 0)
			{
				Sleep(100);
				continue;
			}

			if(!m_pInputStream->IsEOF())
				CLog::Log(LOGINFO, "%s - eof reading from demuxer", __FUNCTION__);

			break;
		}

		// It's a valid data packet, reset error counter
		m_errorCount = 0;

		// Check so that none of our streams has become invalid
		if(!IsValidStream(m_CurrentAudio)    && m_dvdPlayerAudio.IsStalled())    CloseAudioStream(true);
		if(!IsValidStream(m_CurrentVideo)    && m_dvdPlayerVideo.IsStalled())    CloseVideoStream(true);
#ifdef _ENABLE_SUBTITLES
		if(!IsValidStream(m_CurrentSubtitle) && m_dvdPlayerSubtitle.IsStalled()) CloseSubtitleStream(true);
#endif

		// See if we can find something better to play
		if(IsBetterStream(m_CurrentAudio,    pStream)) OpenAudioStream   (pStream->iId, pStream->source);
		if(IsBetterStream(m_CurrentVideo,    pStream)) OpenVideoStream   (pStream->iId, pStream->source);
#ifdef _ENABLE_SUBTITLES
		if(IsBetterStream(m_CurrentSubtitle, pStream)) OpenSubtitleStream(pStream->iId, pStream->source);
#endif
		// Process the packet
		ProcessPacket(pStream, pPacket);

		// Check if in a cut or commercial break that should be automatically skipped
		CheckAutoSceneSkip();
	}
}

void CDVDPlayer::ProcessPacket(CDemuxStream* pStream, DemuxPacket* pPacket)
{
    // Process packet if it belongs to selected stream. for dvd's don't allow automatic opening of streams
    LockStreams();

	try
	{
		if(pPacket->iStreamId == m_CurrentAudio.id && pStream->source == m_CurrentAudio.source && pStream->type == STREAM_AUDIO)
			ProcessAudioData(pStream, pPacket);
		else if(pPacket->iStreamId == m_CurrentVideo.id && pStream->source == m_CurrentVideo.source && pStream->type == STREAM_VIDEO)
			ProcessVideoData(pStream, pPacket);
#ifdef _ENABLE_SUBTITLES
		else if (pPacket->iStreamId == m_CurrentSubtitle.id && pStream->source == m_CurrentSubtitle.source && pStream->type == STREAM_SUBTITLE)
			ProcessSubData(pStream, pPacket);
#endif
		else
		{
			pStream->SetDiscard(AVDISCARD_ALL);
			CDVDDemuxUtils::FreeDemuxPacket(pPacket); // Free it since we won't do anything with it
		}
	}
	catch(...)
	{
		CLog::Log(LOGERROR, "%s - Exception thrown when processing demux packet", __FUNCTION__);
	}

	UnlockStreams();
}

void CDVDPlayer::ProcessAudioData(CDemuxStream* pStream, DemuxPacket* pPacket)
{
	if(m_CurrentAudio.stream != (void*)pStream)
	{
		// Check so that dmuxer hints or extra data hasn't changed
		// if they have, reopen stream
		if(m_CurrentAudio.hint != CDVDStreamInfo(*pStream, true))
			OpenAudioStream( pPacket->iStreamId, pStream->source );

		m_CurrentAudio.stream = (void*)pStream;
	}

	// Check if we are too slow and need to recache
	CheckStartCaching(m_CurrentAudio);

	CheckContinuity(m_CurrentAudio, pPacket);

	if(pPacket->dts != DVD_NOPTS_VALUE)
		m_CurrentAudio.dts = pPacket->dts;
	else if(pPacket->pts != DVD_NOPTS_VALUE)
		m_CurrentAudio.dts = pPacket->pts;

	bool drop = false;
	if(CheckPlayerInit(m_CurrentAudio, DVDPLAYER_AUDIO))
		drop = true;

	if(CheckSceneSkip(m_CurrentAudio))
		drop = true;

	m_dvdPlayerAudio.SendMessage(new CDVDMsgDemuxerPacket(pPacket, drop));
}

void CDVDPlayer::ProcessVideoData(CDemuxStream* pStream, DemuxPacket* pPacket)
{
	if(m_CurrentVideo.stream != (void*)pStream)
	{
		// Check so that dmuxer hints or extra data hasn't
		// changed if they have reopen stream
		if(m_CurrentVideo.hint != CDVDStreamInfo(*pStream, true))
			OpenVideoStream(pPacket->iStreamId, pStream->source);

		m_CurrentVideo.stream = (void*)pStream;
	}

	// Check if we are too slow and need to recache
	CheckStartCaching(m_CurrentVideo);

	if(pPacket->iSize != 4) // Don't check the EOF_SEQUENCE of stillframes
	{
		CheckContinuity(m_CurrentVideo, pPacket);

		if(pPacket->dts != DVD_NOPTS_VALUE)
			m_CurrentVideo.dts = pPacket->dts;
		else if(pPacket->pts != DVD_NOPTS_VALUE)
			m_CurrentVideo.dts = pPacket->pts;
	}

	bool drop = false;

	if(CheckPlayerInit(m_CurrentVideo, DVDPLAYER_VIDEO))
		drop = true;

	if(CheckSceneSkip(m_CurrentVideo))
		drop = true;

	m_dvdPlayerVideo.SendMessage(new CDVDMsgDemuxerPacket(pPacket, drop));
}

void CDVDPlayer::OnExit()
{
	g_dvdPerformanceCounter.DisableMainPerformance();

	try
	{
		CLog::Log(LOGNOTICE, "CDVDPlayer::OnExit()");

		// Set event to inform openfile something went wrong in case openfile is still waiting for this event
		SetCaching(CACHESTATE_DONE);

		// Close each stream
		if(!m_bAbortRequest)
			CLog::Log(LOGNOTICE, "DVDPlayer: eof, waiting for queues to empty");
	
		if(m_CurrentAudio.id >= 0)
		{
			CLog::Log(LOGNOTICE, "DVDPlayer: closing audio stream");
			CloseAudioStream(!m_bAbortRequest);
		}

		if(m_CurrentVideo.id >= 0)
		{
			CLog::Log(LOGNOTICE, "DVDPlayer: closing video stream");
			CloseVideoStream(!m_bAbortRequest);
		}

#ifdef _ENABLE_SUBTITLES
		if(m_CurrentSubtitle.id >= 0)
		{
			CLog::Log(LOGNOTICE, "DVDPlayer: closing subtitle stream");
			CloseSubtitleStream(!m_bAbortRequest);
		}
#endif
		// Destroy the demuxer
		if(m_pDemuxer)
		{
			CLog::Log(LOGNOTICE, "CDVDPlayer::OnExit() deleting demuxer");
			delete m_pDemuxer;
		}
		m_pDemuxer = NULL;

		if(m_pSubtitleDemuxer)
		{
			CLog::Log(LOGNOTICE, "CDVDPlayer::OnExit() deleting subtitle demuxer");
			delete m_pSubtitleDemuxer;
		}
		m_pSubtitleDemuxer = NULL;

		// Destroy the inputstream
		if(m_pInputStream)
		{
			CLog::Log(LOGNOTICE, "CDVDPlayer::OnExit() deleting input stream");
			delete m_pInputStream;
		}
		m_pInputStream = NULL;

		// Clean up all selection streams
		m_SelectionStreams.Clear(STREAM_NONE, STREAM_SOURCE_NONE);

		m_messenger.End();
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s - Exception thrown when trying to close down player, memory leak will follow", __FUNCTION__);
		m_pInputStream = NULL;
		m_pDemuxer = NULL;
	}

	m_bStop = true;

	// If we didn't stop playing, advance to the next item in xbmc's playlist
	if(m_PlayerOptions.identify == false)
	{
		if(m_bAbortRequest)
			m_callback.OnPlayBackStopped();
		else
			m_callback.OnPlayBackEnded();
	}

	// Set event to inform openfile something went wrong 
	// in case openfile is still waiting for this event
	SetEvent(m_hReadyEvent);
}

void CDVDPlayer::SynchronizeDemuxer(DWORD timeout)
{
	if(IsCurrentThread())
		return;

	if(!m_messenger.IsInited())
		return;

	CDVDMsgGeneralSynchronize* message = new CDVDMsgGeneralSynchronize(timeout, 0);
	m_messenger.Put(message->Acquire());
	message->Wait(&m_bStop, 0);
	message->Release();
}

void CDVDPlayer::SynchronizePlayers(DWORD sources, double pts)
{
	// If we are awaiting a start sync, we can't sync here or we could deadlock
	if(m_CurrentAudio.startsync
	|| m_CurrentVideo.startsync
	|| m_CurrentSubtitle.startsync)
	{
		CLog::Log(LOGDEBUG, "%s - can't sync since we are already awaiting a sync", __FUNCTION__);
		return;
	}

	// We need a big timeout as audio queue is about 8seconds for 2ch ac3
	const int timeout = 10*1000; // In milliseconds

	CDVDMsgGeneralSynchronize* message = new CDVDMsgGeneralSynchronize(timeout, sources);
	if(m_CurrentAudio.id >= 0)
	{
		m_CurrentAudio.dts = DVD_NOPTS_VALUE;
		m_CurrentAudio.startpts  = pts;
		m_CurrentAudio.startsync = message->Acquire();
	}

	if(m_CurrentVideo.id >= 0)
	{
		m_CurrentVideo.dts = DVD_NOPTS_VALUE;
		m_CurrentVideo.startpts  = pts;
		m_CurrentVideo.startsync = message->Acquire();
	}

// TODO - we have to rewrite the sync class, to not require
//        all other players waiting for subtitle, should only
//        be the oposite way
/*
	if(m_CurrentSubtitle.id >= 0)
	{
		m_CurrentSubtitle.dts = DVD_NOPTS_VALUE;
		m_CurrentSubtitle.startpts  = pts;
		m_CurrentSubtitle.startsync = message->Acquire();
	}
*/
	message->Release();
}

void CDVDPlayer::CheckAutoSceneSkip()
{
	if(!m_Edl.HasCut())
		return;

	// Check that there is an audio and video stream
	if(m_CurrentAudio.id < 0
	|| m_CurrentVideo.id < 0)
		return;

	// If there is a startpts defined for either the audio or video stream then dvdplayer is still
	// still decoding frames to get to the previously requested seek point
	if(m_CurrentAudio.startpts != DVD_NOPTS_VALUE
	|| m_CurrentVideo.startpts != DVD_NOPTS_VALUE)
		return;

	if(m_CurrentAudio.dts == DVD_NOPTS_VALUE
	|| m_CurrentVideo.dts == DVD_NOPTS_VALUE)
		return;

	const int64_t clock = DVD_TIME_TO_MSEC(min(m_CurrentAudio.dts, m_CurrentVideo.dts));

	CEdl::Cut cut;
	if(!m_Edl.InCut(clock, &cut))
		return;

	if(cut.action == CEdl::CUT
	&& !(cut.end == m_EdlAutoSkipMarkers.cut || cut.start == m_EdlAutoSkipMarkers.cut)) // To prevent looping if same cut again
	{
		CLog::Log(LOGDEBUG, "%s - Clock in EDL cut [%s - %s]: %s. Automatically skipping over.",
				__FUNCTION__, CEdl::MillisecondsToTimeString(cut.start).c_str(),
				CEdl::MillisecondsToTimeString(cut.end).c_str(), CEdl::MillisecondsToTimeString(clock).c_str());

		// Seeking either goes to the start or the end of the cut depending on the play direction
		__int64 seek = GetPlaySpeed() >= 0 ? cut.end : cut.start;

		// Seeking is NOT flushed so any content up to the demux point is retained when playing forwards
		m_messenger.Put(new CDVDMsgPlayerSeek((int)seek, true, false, true, false));

		// Seek doesn't always work reliably. Last physical seek time is recorded to prevent looping
		// if there was an error with seeking and it landed somewhere unexpected, perhaps back in the
		// cut. The cut automatic skip marker is reset every 500ms allowing another attempt at the seek
		m_EdlAutoSkipMarkers.cut = GetPlaySpeed() >= 0 ? cut.end : cut.start;
	}
	else if(cut.action == CEdl::COMM_BREAK
	&&      GetPlaySpeed() >= 0
	&&      cut.start > m_EdlAutoSkipMarkers.commbreak_end)
	{
		CLog::Log(LOGDEBUG, "%s - Clock in commercial break [%s - %s]: %s. Automatically skipping to end of commercial break (only done once per break)",
				__FUNCTION__, CEdl::MillisecondsToTimeString(cut.start).c_str(), CEdl::MillisecondsToTimeString(cut.end).c_str(),
				CEdl::MillisecondsToTimeString(clock).c_str());

		// Seeking is NOT flushed so any content up to the demux point is retained when playing forwards.
		m_messenger.Put(new CDVDMsgPlayerSeek(cut.end + 1, true, false, true, false));

		// Each commercial break is only skipped once so poorly detected commercial breaks can be
		// manually re-entered. Start and end are recorded to prevent looping and to allow seeking back
		// to the start of the commercial break if incorrectly flagged.
		m_EdlAutoSkipMarkers.commbreak_start = cut.start;
		m_EdlAutoSkipMarkers.commbreak_end   = cut.end;
		m_EdlAutoSkipMarkers.seek_to_start   = true; // Allow backwards Seek() to go directly to the start
	}

	// Reset the EDL automatic skip cut marker every 500 ms.
	m_EdlAutoSkipMarkers.ResetCutMarker(500); // in msec
}

void CDVDPlayer::Pause()
{
	if(m_playSpeed != DVD_PLAYSPEED_PAUSE && m_caching == CACHESTATE_FULL)
	{
		SetCaching(CACHESTATE_DONE);
		return;
	}

	// Return to normal speed if it was paused before, pause otherwise
	if(m_playSpeed == DVD_PLAYSPEED_PAUSE)
	{
		SetPlaySpeed(DVD_PLAYSPEED_NORMAL);
		m_callback.OnPlayBackResumed();
	}
	else
	{
		SetPlaySpeed(DVD_PLAYSPEED_PAUSE);
		m_callback.OnPlayBackPaused();
	}
}

bool CDVDPlayer::OnAction(const CAction &action)
{
	return false;
}

void CDVDPlayer::SeekTime(__int64 iTime)
{
	int seekOffset = (int)(iTime - GetTime());
	m_messenger.Put(new CDVDMsgPlayerSeek((int)iTime, true, true, true));
	SynchronizeDemuxer(100);
	m_callback.OnPlayBackSeek((int)iTime, seekOffset);
}

void CDVDPlayer::GetVideoInfo(CStdString& strVideoInfo)
{
	CSingleLock lock(m_StateSection);
	strVideoInfo.Format("D(%s) P(%s)", m_State.demux_video.c_str()
									, m_dvdPlayerVideo.GetPlayerInfo().c_str());
}

void CDVDPlayer::GetAudioInfo(CStdString& strAudioInfo)
{
	CSingleLock lock(m_StateSection);
	strAudioInfo.Format("D(%s) P(%s)", m_State.demux_audio.c_str()
									, m_dvdPlayerAudio.GetPlayerInfo().c_str());
}

void CDVDPlayer::GetGeneralInfo(CStdString& strGeneralInfo)
{
	if(!m_bStop)
	{
		double dDelay = (double)m_dvdPlayerVideo.GetDelay() / DVD_TIME_BASE;

		double apts = m_dvdPlayerAudio.GetCurrentPts();
		double vpts = m_dvdPlayerVideo.GetCurrentPts();
		double dDiff = 0;

		if(apts != DVD_NOPTS_VALUE && vpts != DVD_NOPTS_VALUE)
			dDiff = (apts - vpts) / DVD_TIME_BASE;

		CStdString strEDL;
		strEDL.AppendFormat(", edl:%s", m_Edl.GetInfo().c_str());

		strGeneralInfo.Format("C( ad:% 6.3f, a/v:% 6.3f%s, dcpu:%2i%% acpu:%2i%% vcpu:%2i%% )"
							, dDelay
							, dDiff
							, strEDL.c_str()
							, (int)(CThread::GetRelativeUsage()*100)
							, (int)(m_dvdPlayerAudio.GetRelativeUsage()*100)
							, (int)(m_dvdPlayerVideo.GetRelativeUsage()*100));
	}
}

void CDVDPlayer::CheckContinuity(CCurrentStream& current, DemuxPacket* pPacket)
{
	if(m_playSpeed < DVD_PLAYSPEED_PAUSE)
		return;

	if(pPacket->dts == DVD_NOPTS_VALUE)
		return;

	double mindts, maxdts;

	if(m_CurrentAudio.dts == DVD_NOPTS_VALUE)
		maxdts = mindts = m_CurrentVideo.dts;
	else if(m_CurrentVideo.dts == DVD_NOPTS_VALUE)
		maxdts = mindts = m_CurrentAudio.dts;
	else
	{
		maxdts = max(m_CurrentAudio.dts, m_CurrentVideo.dts);
		mindts = min(m_CurrentAudio.dts, m_CurrentVideo.dts);
	}

	// If we don't have max and min, we can't do anything more
	if(mindts == DVD_NOPTS_VALUE || maxdts == DVD_NOPTS_VALUE)
		return;

	if(pPacket->dts < mindts - DVD_MSEC_TO_TIME(100) && current.inited)
	{
		// If video player is rendering a stillframe, we need to make sure
		// audio has finished processing it's data otherwise it will be
		// displayed too early
		CLog::Log(LOGWARNING, "CDVDPlayer::CheckContinuity - resync backword :%d, prev:%f, curr:%f, diff:%f"
		          , current.type, current.dts, pPacket->dts, pPacket->dts - current.dts);
		
		if(m_dvdPlayerVideo.IsStalled() && m_CurrentVideo.dts != DVD_NOPTS_VALUE)
			SynchronizePlayers(SYNCSOURCE_VIDEO);
		else if(m_dvdPlayerAudio.IsStalled() && m_CurrentAudio.dts != DVD_NOPTS_VALUE)
			SynchronizePlayers(SYNCSOURCE_AUDIO);

		m_CurrentAudio.inited = false;
		m_CurrentVideo.inited = false;
		m_CurrentSubtitle.inited = false;
	}

	// Stream jump forward
	if(pPacket->dts > maxdts + DVD_MSEC_TO_TIME(1000) && current.inited)
	{
		CLog::Log(LOGWARNING, "CDVDPlayer::CheckContinuity - resync forward :%d, prev:%f, curr:%f, diff:%f"
		          , current.type, current.dts, pPacket->dts, pPacket->dts - current.dts);

		// Normally don't need to sync players since video player will keep
		// playing at normal fps after a discontinuity
		//SynchronizePlayers(dts, pts, MSGWAIT_ALL);
		m_CurrentAudio.inited = false;
		m_CurrentVideo.inited = false;
		m_CurrentSubtitle.inited = false;
	}

	if(current.dts != DVD_NOPTS_VALUE && pPacket->dts < current.dts && current.inited)
	{
		// Warn if dts is moving backwords
		CLog::Log(LOGWARNING, "CDVDPlayer::CheckContinuity - wrapback of stream:%d, prev:%f, curr:%f, diff:%f"
		          , current.type, current.dts, pPacket->dts, pPacket->dts - current.dts);
	}
}

void CDVDPlayer::Seek(bool bPlus, bool bLargeStep)
{
#if 0
	// Sadly this doesn't work for now, audio player must
	// drop packets at the same rate as we play frames
	if(m_playSpeed == DVD_PLAYSPEED_PAUSE && bPlus && !bLargeStep)
	{
		m_dvdPlayerVideo.StepFrame();
		return;
	}
#endif
/*
	if(((bPlus && GetChapter() < GetChapterCount()) //TODO
	|| (!bPlus && GetChapter() > 1)) && bLargeStep)
	{
		if(bPlus)
			SeekChapter(GetChapter() + 1);
		else
			SeekChapter(GetChapter() - 1);
		return;
	}
*/
	__int64 seek;
	if(g_advancedSettings.m_videoUseTimeSeeking && GetTotalTime() > 2*g_advancedSettings.m_videoTimeSeekForwardBig)
	{
		if(bLargeStep)
			seek = bPlus ? g_advancedSettings.m_videoTimeSeekForwardBig : g_advancedSettings.m_videoTimeSeekBackwardBig;
		else
			seek = bPlus ? g_advancedSettings.m_videoTimeSeekForward : g_advancedSettings.m_videoTimeSeekBackward;
		
		seek *= 1000;
		seek += GetTime();
	}
	else
	{
		float percent;
		if(bLargeStep)
			percent = bPlus ? g_advancedSettings.m_videoPercentSeekForwardBig : g_advancedSettings.m_videoPercentSeekBackwardBig;
		else
			percent = bPlus ? g_advancedSettings.m_videoPercentSeekForward: g_advancedSettings.m_videoPercentSeekBackward;

		seek = (__int64)(GetTotalTimeInMsec()*(GetPercentage()+percent)/100);
	}

	bool restore = true;

	if(m_Edl.HasCut())
	{
		// Alter the standard seek position based on whether any commercial
		// breaks have been automatically skipped
		const int clock = DVD_TIME_TO_MSEC(m_clock.GetClock());

		// If a large backwards seek occurs within 10 seconds of the end of the last automated
		// commercial skip, then seek back to the start of the commercial break under the assumption
		// it was flagged incorrectly. 10 seconds grace period is allowed in case the watcher has to
		// fumble around finding the remote. Only happens once per commercial break.

		// Small skip does not trigger this in case the start of the commercial break was in fact fine
		// but it skipped too far into the program. In that case small skip backwards behaves as normal.
		if(!bPlus && bLargeStep
		&&  m_EdlAutoSkipMarkers.seek_to_start
		&&  clock >= m_EdlAutoSkipMarkers.commbreak_end
		&&  clock <= m_EdlAutoSkipMarkers.commbreak_end + 10*1000) // Only if within 10 seconds of the end (in msec)
		{
			CLog::Log(LOGDEBUG, "%s - Seeking back to start of commercial break [%s - %s] as large backwards skip activated within 10 seconds of the automatic commercial skip (only done once per break).",
					__FUNCTION__, CEdl::MillisecondsToTimeString(m_EdlAutoSkipMarkers.commbreak_start).c_str(),
					CEdl::MillisecondsToTimeString(m_EdlAutoSkipMarkers.commbreak_end).c_str());

			seek = m_EdlAutoSkipMarkers.commbreak_start;
			restore = false;
			m_EdlAutoSkipMarkers.seek_to_start = false; // So this will only happen within the 10 second grace period once.
		}
		// If big skip forward within the last "reverted" commercial break, seek to the end of the
		// commercial break under the assumption that the break was incorrectly flagged and playback has
		// now reached the actual start of the commercial break. Assume that the end is flagged more
		// correctly than the landing point for a standard big skip (ends seem to be flagged more
		// accurately than the start
		else if(bPlus && bLargeStep
		&&       clock >= m_EdlAutoSkipMarkers.commbreak_start
		&&       clock <= m_EdlAutoSkipMarkers.commbreak_end)
		{
			CLog::Log(LOGDEBUG, "%s - Seeking to end of previously skipped commercial break [%s - %s] as big forwards skip activated within the break.",
					__FUNCTION__, CEdl::MillisecondsToTimeString(m_EdlAutoSkipMarkers.commbreak_start).c_str(),
					CEdl::MillisecondsToTimeString(m_EdlAutoSkipMarkers.commbreak_end).c_str());
			
			seek = m_EdlAutoSkipMarkers.commbreak_end;
			restore = false;
		}
	}

	__int64 time = GetTime();
/*	if(g_application.CurrentFileItem().IsStack()   //TODO
	&&(seek > GetTotalTimeInMsec() || seek < 0))
	{
		g_application.SeekTime((seek - time) * 0.001 + g_application.GetTime());
		// Warning, don't access any dvdplayer variables here as
		// the dvdplayer object may have been destroyed
		return;
	}
*/
	m_messenger.Put(new CDVDMsgPlayerSeek((int)seek, !bPlus, true, false, restore));
	SynchronizeDemuxer(100);

	if(seek < 0) seek = 0;
	m_callback.OnPlayBackSeek((int)seek, (int)(seek - time));
}

void CDVDPlayer::SeekPercentage(float iPercent)
{
	__int64 iTotalTime = GetTotalTimeInMsec();

	if(!iTotalTime)
		return;

	SeekTime((__int64)(iTotalTime * iPercent / 100));
}

float CDVDPlayer::GetPercentage()
{
	__int64 iTotalTime = GetTotalTimeInMsec();

	if(!iTotalTime)
		return 0.0f;

	return GetTime() * 100 / (float)iTotalTime;
}

__int64 CDVDPlayer::GetTime()
{
	CSingleLock lock(m_StateSection);
	double offset = 0;

	if(m_State.timestamp > 0)
	{
		offset  = CDVDClock::GetAbsoluteClock() - m_State.timestamp;
		offset *= m_playSpeed / DVD_PLAYSPEED_NORMAL;
		if(offset >  1000) offset =  1000;
		if(offset < -1000) offset = -1000;
	}

	return (__int64)MathUtils::rint(m_State.time + DVD_TIME_TO_MSEC(offset));
}

__int64 CDVDPlayer::GetTotalTimeInMsec()
{
	CSingleLock lock(m_StateSection);
	return (__int64)MathUtils::rint(m_State.time_total);
}

int CDVDPlayer::GetTotalTime()
{
	return (int)(GetTotalTimeInMsec() / 1000);
}

void CDVDPlayer::FlushBuffers(bool queued)
{
	if(queued)
	{
		m_dvdPlayerAudio.SendMessage(new CDVDMsg(CDVDMsg::GENERAL_RESET));
		m_dvdPlayerVideo.SendMessage(new CDVDMsg(CDVDMsg::GENERAL_RESET));
		m_dvdPlayerVideo.SendMessage(new CDVDMsg(CDVDMsg::VIDEO_NOSKIP));
#ifdef _ENABLE_SUBTITLES
		m_dvdPlayerSubtitle.SendMessage(new CDVDMsg(CDVDMsg::GENERAL_RESET));
#endif
		SynchronizePlayers(SYNCSOURCE_ALL);
	}
	else
	{
		m_dvdPlayerAudio.Flush();
		m_dvdPlayerVideo.Flush();
#ifdef _ENABLE_SUBTITLES
		m_dvdPlayerSubtitle.Flush();
#endif

		// Clear subtitle and menu overlays
//		m_overlayContainer.Clear(); //TODO

		// Make sure players are properly flushed, should put them in stalled state
		CDVDMsgGeneralSynchronize* msg = new CDVDMsgGeneralSynchronize(1000, 0);
		m_dvdPlayerAudio.m_messageQueue.Put(msg->Acquire(), 1);
		m_dvdPlayerVideo.m_messageQueue.Put(msg->Acquire(), 1);
		msg->Wait(&m_bStop, 0);
		msg->Release();

		// Purge any pending PLAYER_STARTED messages
		m_messenger.Flush(CDVDMsg::PLAYER_STARTED);

		// We should now wait for init cache
		SetCaching(CACHESTATE_INIT);
		m_CurrentAudio.started    = false;
		m_CurrentVideo.started    = false;
		m_CurrentSubtitle.started = false;
	}

	m_CurrentAudio.inited = false;
	m_CurrentVideo.inited = false;
	m_CurrentSubtitle.inited = false;  
}

void CDVDPlayer::SetAVDelay(float fValue)
{
	m_dvdPlayerVideo.SetDelay((fValue * DVD_TIME_BASE));
}

bool CDVDPlayer::IsInMenu() const //TODO
{
/*	if(m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
	{
		CDVDInputStreamNavigator* pStream = (CDVDInputStreamNavigator*)m_pInputStream;
		if(m_dvd.state == DVDSTATE_STILL)
			return true;
		else
			return pStream->IsInMenu();
	}
*/	return false;
}

void CDVDPlayer::HandlePlaySpeed()
{
	if(IsInMenu() && m_caching != CACHESTATE_DONE)
		SetCaching(CACHESTATE_DONE);

	if(m_caching == CACHESTATE_INIT)
	{
		// If all enabled streams have been inited we are done
		if((m_CurrentVideo.id < 0 || m_CurrentVideo.inited)
		&& (m_CurrentAudio.id < 0 || m_CurrentAudio.inited))
			SetCaching(CACHESTATE_PLAY);
	}

	if(m_caching == CACHESTATE_PLAY)
	{
		// If all enabled streams have started playing we are done
		if((m_CurrentVideo.id < 0 || !m_dvdPlayerVideo.IsStalled())
		&& (m_CurrentAudio.id < 0 || !m_dvdPlayerAudio.IsStalled()))
			SetCaching(CACHESTATE_DONE);
	}

	if(GetPlaySpeed() != DVD_PLAYSPEED_NORMAL && GetPlaySpeed() != DVD_PLAYSPEED_PAUSE)
	{
		if(IsInMenu())
		{
			// This can't be done in menu
			SetPlaySpeed(DVD_PLAYSPEED_NORMAL);

		}
		else if(m_CurrentVideo.id >= 0
				&&  m_CurrentVideo.inited == true
				&&  m_SpeedState.lastpts  != m_dvdPlayerVideo.GetCurrentPts()
				&&  m_SpeedState.lasttime != GetTime())
		{
			m_SpeedState.lastpts  = m_dvdPlayerVideo.GetCurrentPts();
			m_SpeedState.lasttime = GetTime();
			// Check how much off clock video is when ff/rw:ing
			// a problem here is that seeking isn't very accurate
			// and since the clock will be resynced after seek
			// we might actually not really be playing at the wanted
			// speed. we'd need to have some way to not resync the clock
			// after a seek to remember timing. still need to handle
			// discontinuities somehow

			// When seeking, give the player a headstart to make sure
			// the time it takes to seek doesn't make a difference.
			double error;
			error  = m_clock.GetClock() - m_SpeedState.lastpts;
			error *= m_playSpeed / abs(m_playSpeed);

			if(error > DVD_MSEC_TO_TIME(1000))
			{
				CLog::Log(LOGDEBUG, "CDVDPlayer::Process - Seeking to catch up");
				__int64 iTime = (__int64)DVD_TIME_TO_MSEC(m_clock.GetClock() + m_State.time_offset + 500000.0 * m_playSpeed / DVD_PLAYSPEED_NORMAL);
				m_messenger.Put(new CDVDMsgPlayerSeek(iTime, (GetPlaySpeed() < 0), true, false));
			}
		}
	}
}

void CDVDPlayer::HandleMessages()
{
	CDVDMsg* pMsg;
	LockStreams();

	while(m_messenger.Get(&pMsg, 0) == MSGQ_OK)
	{
		try
		{
			if(pMsg->IsType(CDVDMsg::PLAYER_SEEK))
			{
				CDVDMsgPlayerSeek &msg(*((CDVDMsgPlayerSeek*)pMsg));

//				g_infoManager.SetDisplayAfterSeek(100000); //TODO
				
				if(msg.GetFlush())
					SetCaching(CACHESTATE_INIT);

				double start = DVD_NOPTS_VALUE;

				int time = msg.GetRestore() ? (int)m_Edl.RestoreCutTime(msg.GetTime()) : msg.GetTime();
				CLog::Log(LOGDEBUG, "demuxer seek to: %d", time);

				if(m_pDemuxer && m_pDemuxer->SeekTime(time, msg.GetBackward(), &start))
				{
					CLog::Log(LOGDEBUG, "demuxer seek to: %d, success", time);
					if(m_pSubtitleDemuxer)
					{
						if(!m_pSubtitleDemuxer->SeekTime(time, msg.GetBackward()))
							CLog::Log(LOGDEBUG, "failed to seek subtitle demuxer: %d, success", time);
					}
					
					FlushBuffers(!msg.GetFlush());

					if(msg.GetAccurate())
						SynchronizePlayers(SYNCSOURCE_ALL, start);
					else
						SynchronizePlayers(SYNCSOURCE_ALL, DVD_NOPTS_VALUE);
				}
				else
					CLog::Log(LOGWARNING, "error while seeking");

				// Set flag to indicate we have finished a seeking request
//				g_infoManager.m_performingSeek = false;  //TODO
//				g_infoManager.SetDisplayAfterSeek();  //TODO
			}
			else if(pMsg->IsType(CDVDMsg::PLAYER_SEEK_CHAPTER))
			{
/*				g_infoManager.SetDisplayAfterSeek(100000);
				SetCaching(CACHESTATE_INIT);

				CDVDMsgPlayerSeekChapter &msg(*((CDVDMsgPlayerSeekChapter*)pMsg));
				double start = DVD_NOPTS_VALUE;

				// This should always be the case.
				if(m_pDemuxer && m_pDemuxer->SeekChapter(msg.GetChapter(), &start))
				{
					FlushBuffers(false);
					SynchronizePlayers(SYNCSOURCE_ALL, start);
					m_callback.OnPlayBackSeekChapter(msg.GetChapter());
				}

				g_infoManager.SetDisplayAfterSeek();
*/			}
			else if(pMsg->IsType(CDVDMsg::DEMUXER_RESET))
			{
				m_CurrentAudio.stream = NULL;
				m_CurrentVideo.stream = NULL;
				m_CurrentSubtitle.stream = NULL;

				// We need to reset the demuxer, probably because the streams have changed
				if(m_pDemuxer)
					m_pDemuxer->Reset();
#ifdef _ENABLE_SUBTITLES
				if(m_pSubtitleDemuxer)
					m_pSubtitleDemuxer->Reset();
#endif
			}
			else if(pMsg->IsType(CDVDMsg::PLAYER_SET_AUDIOSTREAM))
			{
/*				CDVDMsgPlayerSetAudioStream* pMsg2 = (CDVDMsgPlayerSetAudioStream*)pMsg;

				SelectionStream& st = m_SelectionStreams.Get(STREAM_AUDIO, pMsg2->GetStreamId());
				if(st.source != STREAM_SOURCE_NONE)
				{
					if(st.source == STREAM_SOURCE_NAV && m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
					{
						CDVDInputStreamNavigator* pStream = (CDVDInputStreamNavigator*)m_pInputStream;
						if(pStream->SetActiveAudioStream(st.id))
						{
							m_dvd.iSelectedAudioStream = -1;
							CloseAudioStream(false);
						}
					}
					else
					{
						CloseAudioStream(false);
						OpenAudioStream(st.id, st.source);
					}
				}
*/			}
			else if(pMsg->IsType(CDVDMsg::PLAYER_SET_SUBTITLESTREAM))
			{
/*				CDVDMsgPlayerSetSubtitleStream* pMsg2 = (CDVDMsgPlayerSetSubtitleStream*)pMsg;

				SelectionStream& st = m_SelectionStreams.Get(STREAM_SUBTITLE, pMsg2->GetStreamId());
				if(st.source != STREAM_SOURCE_NONE)
				{
					if(st.source == STREAM_SOURCE_NAV && m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
					{
						CDVDInputStreamNavigator* pStream = (CDVDInputStreamNavigator*)m_pInputStream;
						if(pStream->SetActiveSubtitleStream(st.id))
						{
							m_dvd.iSelectedSPUStream = -1;
							CloseSubtitleStream(false);
						}
					}
					else
					{
						CloseSubtitleStream(false);
						OpenSubtitleStream(st.id, st.source);
					}
				}
*/			}
			else if(pMsg->IsType(CDVDMsg::PLAYER_SET_SUBTITLESTREAM_VISIBLE))
			{
/*				CDVDMsgBool* pValue = (CDVDMsgBool*)pMsg;

				m_dvdPlayerVideo.EnableSubtitle(pValue->m_value);

				if(m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
					static_cast<CDVDInputStreamNavigator*>(m_pInputStream)->EnableSubtitleStream(pValue->m_value);
*/			}
			else if(pMsg->IsType(CDVDMsg::PLAYER_SET_STATE))
			{
/*				g_infoManager.SetDisplayAfterSeek(100000);
				SetCaching(CACHESTATE_INIT);

				CDVDMsgPlayerSetState* pMsgPlayerSetState = (CDVDMsgPlayerSetState*)pMsg;

				if(m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
				{
					std::string s = pMsgPlayerSetState->GetState();
					((CDVDInputStreamNavigator*)m_pInputStream)->SetNavigatorState(s);
					m_dvd.state = DVDSTATE_NORMAL;
					m_dvd.iDVDStillStartTime = 0;
					m_dvd.iDVDStillTime = 0;
				}

				g_infoManager.SetDisplayAfterSeek();
*/			}
			else if(pMsg->IsType(CDVDMsg::PLAYER_SET_RECORD))
			{
/*				if(m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_TV))
					static_cast<CDVDInputStreamTV*>(m_pInputStream)->Record(*(CDVDMsgBool*)pMsg);
*/			}
			else if(pMsg->IsType(CDVDMsg::GENERAL_FLUSH))
			{
				FlushBuffers(false);
			}
			else if(pMsg->IsType(CDVDMsg::PLAYER_SETSPEED))
			{
				int speed = static_cast<CDVDMsgInt*>(pMsg)->m_value;

				// Correct our current clock, as it would start going wrong otherwise
				if(m_State.timestamp > 0)
				{
					double offset;
					offset  = CDVDClock::GetAbsoluteClock() - m_State.timestamp;
					offset *= m_playSpeed / DVD_PLAYSPEED_NORMAL;
					if(offset >  1000) offset =  1000;
					if(offset < -1000) offset = -1000;
					m_State.time     += DVD_TIME_TO_MSEC(offset);
					m_State.timestamp =  CDVDClock::GetAbsoluteClock();
				}

				if(speed != DVD_PLAYSPEED_PAUSE)
				{
					if(m_playSpeed != DVD_PLAYSPEED_PAUSE)
						m_callback.OnPlayBackSpeedChanged(speed / DVD_PLAYSPEED_NORMAL);
				}

				// If playspeed is different then DVD_PLAYSPEED_NORMAL or DVD_PLAYSPEED_PAUSE
				// audioplayer, stops outputing audio to audiorendere, but still tries to
				// sleep an correct amount for each packet
				// videoplayer just plays faster after the clock speed has been increased
				// 1. disable audio
				// 2. skip frames and adjust their pts or the clock
				m_playSpeed = speed;
				m_caching = CACHESTATE_DONE;
				m_clock.SetSpeed(speed);
				m_dvdPlayerAudio.SetSpeed(speed);
				m_dvdPlayerVideo.SetSpeed(speed);

				// TODO - we really shouldn't pause demuxer
				//        until our buffers are somewhat filled
				if(m_pDemuxer)
					m_pDemuxer->SetSpeed(speed);
			}
			else if(pMsg->IsType(CDVDMsg::PLAYER_CHANNEL_NEXT) ||
					pMsg->IsType(CDVDMsg::PLAYER_CHANNEL_PREV) ||
					pMsg->IsType(CDVDMsg::PLAYER_CHANNEL_SELECT))
			{
/*				CDVDInputStream::IChannel* input = dynamic_cast<CDVDInputStream::IChannel*>(m_pInputStream);
				if(input)
				{
					g_infoManager.SetDisplayAfterSeek(100000);

					bool result;
					if (pMsg->IsType(CDVDMsg::PLAYER_CHANNEL_SELECT))
						result = input->SelectChannel(static_cast<CDVDMsgInt*>(pMsg)->m_value);
					else if(pMsg->IsType(CDVDMsg::PLAYER_CHANNEL_NEXT))
						result = input->NextChannel();
					else
						result = input->PrevChannel();

					if(result)
					{
						FlushBuffers(false);
						SAFE_DELETE(m_pDemuxer);
					}

					g_infoManager.SetDisplayAfterSeek();
				}
	*/		}
			else if(pMsg->IsType(CDVDMsg::GENERAL_GUI_ACTION))
				OnAction(((CDVDMsgType<CAction>*)pMsg)->m_value);
			else if(pMsg->IsType(CDVDMsg::PLAYER_STARTED))
			{
				int player = ((CDVDMsgInt*)pMsg)->m_value;

				if(player == DVDPLAYER_AUDIO)
					m_CurrentAudio.started = true;
				if(player == DVDPLAYER_VIDEO)
					m_CurrentVideo.started = true;

				CLog::Log(LOGDEBUG, "CDVDPlayer::HandleMessages - player started %d", player);
			}
		}
		catch (...)
		{
			CLog::Log(LOGERROR, "%s - Exception thrown when handling message", __FUNCTION__);
		}

		pMsg->Release();
	}
	UnlockStreams();
}

bool CDVDPlayer::OpenInputStream()
{
	if(m_pInputStream)
		SAFE_DELETE(m_pInputStream);

	CLog::Log(LOGNOTICE, "Creating InputStream");

	// Correct the filename if needed
	CStdString filename(m_filename);
	if(filename.Find("dvd://") == 0
	   || filename.CompareNoCase("d:\\video_ts\\video_ts.ifo") == 0
	   || filename.CompareNoCase("iso9660://video_ts/video_ts.ifo") == 0)
	{
#ifdef _WIN32PC
		m_filename = MEDIA_DETECT::CLibcdio::GetInstance()->GetDeviceFileName()+4;
#elif defined (_LINUX)
		m_filename = MEDIA_DETECT::CLibcdio::GetInstance()->GetDeviceFileName();
#else
		m_filename = "\\Device\\Cdrom0";
#endif
	}

	m_pInputStream = CDVDFactoryInputStream::CreateInputStream(this, m_filename, m_mimetype);
	if(m_pInputStream == NULL)
	{
		CLog::Log(LOGERROR, "CDVDPlayer::OpenInputStream - unable to create input stream for [%s]", m_filename.c_str());
		return false;
	}
	else
		m_pInputStream->SetFileItem(m_item);

	if(!m_pInputStream->Open(m_filename.c_str(), m_mimetype))
	{
		CLog::Log(LOGERROR, "CDVDPlayer::OpenInputStream - error opening [%s]", m_filename.c_str());
		return false;
	}

	// Find any available external subtitles for non dvd files
	if(!m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD)
	&& !m_pInputStream->IsStreamType(DVDSTREAM_TYPE_TV)
	&& !m_pInputStream->IsStreamType(DVDSTREAM_TYPE_HTSP))
	{
#ifdef _ENABLE_SUBTITLES
		// Find any available external subtitles
		std::vector<std::string> filenames;
		CDVDFactorySubtitle::GetSubtitles(filenames, m_filename);

		// Find any upnp subtitles
		CStdString key("upnp:subtitle:1");
		for(unsigned s = 1; m_item.HasProperty(key); key.Format("upnp:subtitle:%u", ++s))
			filenames.push_back(m_item.GetProperty(key));

		for(unsigned int i=0;i<filenames.size();i++)
			AddSubtitleFile(filenames[i], i == 0 ? CDemuxStream::FLAG_DEFAULT : CDemuxStream::FLAG_NONE);

		g_stSettings.m_currentVideoSettings.m_SubtitleCached = true;
#endif
	}

	SetAVDelay(g_settings.m_currentVideoSettings.m_AudioDelay);
#ifdef _ENABLE_SUBTITLES
	SetSubTitleDelay(g_stSettings.m_currentVideoSettings.m_SubtitleDelay);
#endif
	m_clock.Discontinuity(CLOCK_DISC_FULL);
	m_dvd.Clear();
	m_errorCount = 0;

	return true;
}

bool CDVDPlayer::OpenDemuxStream()
{
	if(m_pDemuxer)
		SAFE_DELETE(m_pDemuxer);

	CLog::Log(LOGNOTICE, "Creating Demuxer");

	try
	{
		int attempts = 10;
		while(!m_bStop && attempts-- > 0)
		{
			m_pDemuxer = CDVDFactoryDemuxer::CreateDemuxer(m_pInputStream);
			if(!m_pDemuxer && m_pInputStream->NextStream())
			{
				CLog::Log(LOGDEBUG, "%s - New stream available from input, retry open", __FUNCTION__);
				continue;
			}
			break;
		}

		if(!m_pDemuxer)
		{
			CLog::Log(LOGERROR, "%s - Error creating demuxer", __FUNCTION__);
			return false;
		}
	}
	catch(...)
	{
		CLog::Log(LOGERROR, "%s - Exception thrown when opening demuxer", __FUNCTION__);
		return false;
	}

	m_SelectionStreams.Clear(STREAM_NONE, STREAM_SOURCE_DEMUX);
	m_SelectionStreams.Clear(STREAM_NONE, STREAM_SOURCE_NAV);
	m_SelectionStreams.Update(m_pInputStream, m_pDemuxer);

	return true;
}

void CDVDPlayer::OpenDefaultStreams()
{
	int  count;
	bool valid;
	bool force = false;
	SelectionStream st;

	// Open video stream
	count = m_SelectionStreams.Count(STREAM_VIDEO);
	valid = false;

	if(!valid && m_SelectionStreams.Get(STREAM_VIDEO, CDemuxStream::FLAG_DEFAULT, st))
	{
		if(OpenVideoStream(st.id, st.source))
			valid = true;
		else
			CLog::Log(LOGWARNING, "%s - failed to open default stream (%d)", __FUNCTION__, st.id);
	}

	for(int i = 0;i<count && !valid;i++)
	{
		SelectionStream& s = m_SelectionStreams.Get(STREAM_VIDEO, i);
		if(OpenVideoStream(s.id, s.source))
			valid = true;
	}
	if(!valid)
		CloseVideoStream(true);

	if(!m_PlayerOptions.video_only)
	{
		// Open audio stream
		count = m_SelectionStreams.Count(STREAM_AUDIO);
		valid = false;
		if(g_settings.m_currentVideoSettings.m_AudioStream >= 0
		   && g_settings.m_currentVideoSettings.m_AudioStream < count)
		{
			SelectionStream& s = m_SelectionStreams.Get(STREAM_AUDIO, g_settings.m_currentVideoSettings.m_AudioStream);
			if(OpenAudioStream(s.id, s.source))
				valid = true;
			else
				CLog::Log(LOGWARNING, "%s - failed to restore selected audio stream (%d)", __FUNCTION__, g_settings.m_currentVideoSettings.m_AudioStream);
		}

		if(!valid && m_SelectionStreams.Get(STREAM_AUDIO, CDemuxStream::FLAG_DEFAULT, st))
		{
			if(OpenAudioStream(st.id, st.source))
				valid = true;
			else
				CLog::Log(LOGWARNING, "%s - failed to open default stream (%d)", __FUNCTION__, st.id);
		}

		for(int i = 0; i<count && !valid; i++)
		{
			SelectionStream& s = m_SelectionStreams.Get(STREAM_AUDIO, i);
			if(OpenAudioStream(s.id, s.source))
				valid = true;
		}
		if(!valid)
			CloseAudioStream(true);
	}

#ifdef _ENABLE_SUBTITLES
	// Open subtitle stream
	count = m_SelectionStreams.Count(STREAM_SUBTITLE);
	valid = false;

	// If subs are disabled, check for forced
	if(!valid && !g_stSettings.m_currentVideoSettings.m_SubtitleOn 
	   && m_SelectionStreams.Get(STREAM_SUBTITLE, CDemuxStream::FLAG_FORCED, st))
	{
		if(OpenSubtitleStream(st.id, st.source))
		{
			valid = true;
			force = true;
		}
		else
			CLog::Log(LOGWARNING, "%s - failed to open default/forced stream (%d)", __FUNCTION__, st.id);
	}

	// Restore selected
	if(!valid
	   && g_stSettings.m_currentVideoSettings.m_SubtitleStream >= 0
	   && g_stSettings.m_currentVideoSettings.m_SubtitleStream < count)
	{
		SelectionStream& s = m_SelectionStreams.Get(STREAM_SUBTITLE, g_stSettings.m_currentVideoSettings.m_SubtitleStream);
		if(OpenSubtitleStream(s.id, s.source))
			valid = true;
		else
			CLog::Log(LOGWARNING, "%s - failed to restore selected subtitle stream (%d)", __FUNCTION__, g_stSettings.m_currentVideoSettings.m_SubtitleStream);
	}

	// Select default
	if(!valid && m_SelectionStreams.Get(STREAM_SUBTITLE, CDemuxStream::FLAG_DEFAULT, st))
	{
		if(OpenSubtitleStream(st.id, st.source))
			valid = true;
		else
			CLog::Log(LOGWARNING, "%s - failed to open default/forced stream (%d)", __FUNCTION__, st.id);
	}

	// Select first
	for(int i = 0;i<count && !valid; i++)
	{
		SelectionStream& s = m_SelectionStreams.Get(STREAM_SUBTITLE, i);
		if(OpenSubtitleStream(s.id, s.source))
			valid = true;
	}
	if(!valid)
		CloseSubtitleStream(false);

	if((g_stSettings.m_currentVideoSettings.m_SubtitleOn || force) && !m_PlayerOptions.video_only)
		m_dvdPlayerVideo.EnableSubtitle(true);
	else
		m_dvdPlayerVideo.EnableSubtitle(false);
#endif
}

bool CDVDPlayer::OpenAudioStream(int iStream, int source)
{
	CLog::Log(LOGNOTICE, "Opening audio stream: %i source: %i", iStream, source);

	if(!m_pDemuxer)
		return false;

	CDemuxStream* pStream = m_pDemuxer->GetStream(iStream);
	if(!pStream || pStream->disabled)
		return false;

	if(m_CurrentAudio.id < 0 &&  m_CurrentVideo.id >= 0)
	{
		// Up until now we wheren't playing audio, but we did play video
		// this will change what is used to sync the dvdclock.
		// since the new audio data doesn't have to have any relation
		// to the current video data in the packet que, we have to
		// wait for it to empty

		// this happens if a new cell has audio data, but previous didn't
		// and both have video data
		SynchronizePlayers(SYNCSOURCE_AUDIO);
	}

	CDVDStreamInfo hint(*pStream, true);

	if(m_CurrentAudio.id < 0 || m_CurrentAudio.hint != hint)
	{
		if(!m_dvdPlayerAudio.OpenStream( hint ))
		{
			// Mark stream as disabled, to disallaw further attempts
			CLog::Log(LOGWARNING, "%s - Unsupported stream %d. Stream disabled.", __FUNCTION__, iStream);
			pStream->disabled = true;
			pStream->SetDiscard(AVDISCARD_ALL);
			return false;
		}
	}
	else
		m_dvdPlayerAudio.SendMessage(new CDVDMsg(CDVDMsg::GENERAL_RESET));

	// Store information about stream
	m_CurrentAudio.id = iStream;
	m_CurrentAudio.source = source;
	m_CurrentAudio.hint = hint;
	m_CurrentAudio.stream = (void*)pStream;
	m_CurrentAudio.started = false;

	// Audio normally won't consume full cpu, so let it have prioity
	m_dvdPlayerAudio.SetPriority(GetThreadPriority(*this)+1);

	return true;
}

bool CDVDPlayer::OpenVideoStream(int iStream, int source)
{
	CLog::Log(LOGNOTICE, "Opening video stream: %i source: %i", iStream, source);

	if(!m_pDemuxer)
		return false;

	CDemuxStream* pStream = m_pDemuxer->GetStream(iStream);
	
	if(!pStream || pStream->disabled)
		return false;

	pStream->SetDiscard(AVDISCARD_NONE);

	CDVDStreamInfo hint(*pStream, true);

	if(m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
	{
/*		// Set aspect ratio as requested by navigator for dvd's
		float aspect = static_cast<CDVDInputStreamNavigator*>(m_pInputStream)->GetVideoAspectRatio();
		
		if(aspect != 0.0)
			hint.aspect = aspect;

		hint.stills   = true;
*/	}

	if(m_CurrentVideo.id < 0  || m_CurrentVideo.hint != hint)
	{
		if(!m_dvdPlayerVideo.OpenStream(hint))
		{
			// Mark stream as disabled, to disallaw further attempts
			CLog::Log(LOGWARNING, "%s - Unsupported stream %d. Stream disabled.", __FUNCTION__, iStream);
			pStream->disabled = true;
			pStream->SetDiscard(AVDISCARD_ALL);
			return false;
		}
	}
	else
		m_dvdPlayerVideo.SendMessage(new CDVDMsg(CDVDMsg::GENERAL_RESET));

	// Store information about stream
	m_CurrentVideo.id = iStream;
	m_CurrentVideo.source = source;
	m_CurrentVideo.hint = hint;
	m_CurrentVideo.stream = (void*)pStream;
	m_CurrentVideo.started = false;

	// Use same priority for video thread as demuxing thread, as
	// otherwise demuxer will starve if video consumes the full cpu
	m_dvdPlayerVideo.SetPriority(GetThreadPriority(*this));

	return true;
}

bool CDVDPlayer::CloseAudioStream(bool bWaitForBuffers)
{
	if(m_CurrentAudio.id < 0)
		return false;

	CLog::Log(LOGNOTICE, "Closing audio stream");

	if(bWaitForBuffers)
		SetCaching(CACHESTATE_DONE);

	m_dvdPlayerAudio.CloseStream(bWaitForBuffers);

	m_CurrentAudio.Clear();
	return true;
}

bool CDVDPlayer::CloseVideoStream(bool bWaitForBuffers)
{
	if(m_CurrentVideo.id < 0)
		return false;

	CLog::Log(LOGNOTICE, "Closing video stream");

	if(bWaitForBuffers)
		SetCaching(CACHESTATE_DONE);

	m_dvdPlayerVideo.CloseStream(bWaitForBuffers);

	m_CurrentVideo.Clear();
	return true;
}

void CDVDPlayer::SetPlaySpeed(int speed)
{
	m_messenger.Put(new CDVDMsgInt(CDVDMsg::PLAYER_SETSPEED, speed));
	m_dvdPlayerAudio.SetSpeed(speed);
	m_dvdPlayerVideo.SetSpeed(speed);
	SynchronizeDemuxer(100);
}

void CDVDPlayer::SetCaching(ECacheState state)
{
	if(m_caching == state)
		return;

	CLog::Log(LOGDEBUG, "CDVDPlayer::SetCaching - caching state %d", state);

	if(state == CACHESTATE_FULL || state == CACHESTATE_INIT)
	{
		m_clock.SetSpeed(DVD_PLAYSPEED_PAUSE);
		m_dvdPlayerAudio.SetSpeed(DVD_PLAYSPEED_PAUSE);
		m_dvdPlayerAudio.SendMessage(new CDVDMsg(CDVDMsg::PLAYER_STARTED), 1);
		m_dvdPlayerVideo.SetSpeed(DVD_PLAYSPEED_PAUSE);
		m_dvdPlayerVideo.SendMessage(new CDVDMsg(CDVDMsg::PLAYER_STARTED), 1);
	}

	if(state == CACHESTATE_PLAY ||(state == CACHESTATE_DONE && m_caching != CACHESTATE_PLAY))
	{
		m_clock.SetSpeed(m_playSpeed);
		m_dvdPlayerAudio.SetSpeed(m_playSpeed);
		m_dvdPlayerVideo.SetSpeed(m_playSpeed);
	}

	m_caching = state;
}

bool CDVDPlayer::CheckStartCaching(CCurrentStream& current)
{
	if(m_caching != CACHESTATE_DONE || m_playSpeed != DVD_PLAYSPEED_NORMAL)
		return false;

	if(IsInMenu())
		return false;

	if((current.type == STREAM_AUDIO && m_dvdPlayerAudio.IsStalled())
	|| (current.type == STREAM_VIDEO && m_dvdPlayerVideo.IsStalled()))
	{
		// Don't start caching if it's only a single stream that has run dry
		if(m_dvdPlayerAudio.m_messageQueue.GetLevel() > 50
		|| m_dvdPlayerVideo.m_messageQueue.GetLevel() > 50)
			return false;

		if(m_pInputStream->IsStreamType(DVDSTREAM_TYPE_HTSP)
		   || m_pInputStream->IsStreamType(DVDSTREAM_TYPE_TV))
			SetCaching(CACHESTATE_INIT);
		else
		{
			if(current.inited)
				SetCaching(CACHESTATE_FULL);
			else
				SetCaching(CACHESTATE_INIT);
		}
		return true;
	}
	return false;
}

void CDVDPlayer::SendPlayerMessage(CDVDMsg* pMsg, unsigned int target)
{
	if(target == DVDPLAYER_AUDIO)
		m_dvdPlayerAudio.SendMessage(pMsg);

	if(target == DVDPLAYER_VIDEO)
		m_dvdPlayerVideo.SendMessage(pMsg);

#ifdef _ENABLE_SUBTITLES
	if(target == DVDPLAYER_SUBTITLE)
		m_dvdPlayerSubtitle.SendMessage(pMsg);
#endif
}

bool CDVDPlayer::CheckSceneSkip(CCurrentStream& current)
{
	if(!m_Edl.HasCut())
		return false;

	if(current.dts == DVD_NOPTS_VALUE)
		return false;

	if(current.startpts != DVD_NOPTS_VALUE)
		return false;

	CEdl::Cut cut;
	return m_Edl.InCut(DVD_TIME_TO_MSEC(current.dts), &cut) && cut.action == CEdl::CUT;
}

bool CDVDPlayer::CheckPlayerInit(CCurrentStream& current, unsigned int source)
{
	if(current.startsync)
	{
		if((current.startpts < current.dts && current.dts != DVD_NOPTS_VALUE)
		   || (current.startpts == DVD_NOPTS_VALUE))
		{
			SendPlayerMessage(current.startsync, source);

			current.startpts = DVD_NOPTS_VALUE;
			current.startsync = NULL;
		}
		else if((current.startpts - current.dts) > DVD_SEC_TO_TIME(20)
		        &&  current.dts != DVD_NOPTS_VALUE)
		{
			CLog::Log(LOGDEBUG, "%s - too far to decode before finishing seek", __FUNCTION__);
			
			if(m_CurrentAudio.startpts != DVD_NOPTS_VALUE)
				m_CurrentAudio.startpts = current.dts;

			if(m_CurrentVideo.startpts != DVD_NOPTS_VALUE)
				m_CurrentVideo.startpts = current.dts;

			if(m_CurrentSubtitle.startpts != DVD_NOPTS_VALUE)
				m_CurrentSubtitle.startpts = current.dts;
		}
	}

	// Await start sync to be finished
	if(current.startsync)
	{
		CLog::Log(LOGDEBUG, "%s - dropping packet type:%d dts:%f to get to start point at %f", __FUNCTION__, source,  current.dts, current.startpts);
		return true;
	}

	// If this is the first packet after a discontinuity, send it as a resync
	if(current.inited == false && current.dts != DVD_NOPTS_VALUE)
	{
		current.inited   = true;
		current.startpts = current.dts;

		bool setclock = false;
		if(m_playSpeed == DVD_PLAYSPEED_NORMAL)
		{
			if(source == DVDPLAYER_AUDIO)
				setclock = !m_CurrentVideo.inited;
			else if(source == DVDPLAYER_VIDEO)
				setclock = !m_CurrentAudio.inited;
		}
		else
		{
			if(source == DVDPLAYER_VIDEO)
				setclock = true;
		}

		double starttime = current.startpts;
		if(m_CurrentAudio.inited
		   && m_CurrentAudio.startpts != DVD_NOPTS_VALUE
		   && m_CurrentAudio.startpts < starttime)
				starttime = m_CurrentAudio.startpts;

		if(m_CurrentVideo.inited
		   && m_CurrentVideo.startpts != DVD_NOPTS_VALUE
		   && m_CurrentVideo.startpts < starttime)
				starttime = m_CurrentVideo.startpts;

		starttime = current.startpts - starttime;
		if(starttime > 0)
		{
			if(starttime > DVD_SEC_TO_TIME(2))
				CLog::Log(LOGWARNING, "CDVDPlayer::CheckPlayerInit(%d) - Ignoring too large delay of %f", source, starttime);
			else
				SendPlayerMessage(new CDVDMsgDouble(CDVDMsg::GENERAL_DELAY, starttime), source);
		}
		SendPlayerMessage(new CDVDMsgGeneralResync(current.dts, setclock), source);
	}
	return false;
}

bool CDVDPlayer::ReadPacket(DemuxPacket*& packet, CDemuxStream*& stream)
{
#ifdef _ENABLE_SUBTITLES
	// Check if we should read from subtitle demuxer
	if(m_dvdPlayerSubtitle.AcceptsData() && m_pSubtitleDemuxer )
	{
		if(m_pSubtitleDemuxer)
			packet = m_pSubtitleDemuxer->Read();

		if(packet)
		{
			if(packet->iStreamId < 0)
				return true;

			stream = m_pSubtitleDemuxer->GetStream(packet->iStreamId);
			if(!stream)
			{
				CLog::Log(LOGERROR, "%s - Error demux packet doesn't belong to a valid stream", __FUNCTION__);
				return false;
			}

			if(stream->source == STREAM_SOURCE_NONE)
			{
				m_SelectionStreams.Clear(STREAM_NONE, STREAM_SOURCE_DEMUX_SUB);
				m_SelectionStreams.Update(NULL, m_pSubtitleDemuxer);
			}
			return true;
		}
	}
#endif

	// Read a data frame from stream.
	if(m_pDemuxer)
		packet = m_pDemuxer->Read();

	if(packet)
	{
		// This groupId stuff is getting a bit messy, need to find a better way
		// currently it is used to determine if a menu overlay is associated with a picture
		// for dvd's we use as a group id, the current cell and the current title
		// to be a bit more precise we alse count the number of disc's in case of a pts wrap back in the same cell / title
		packet->iGroupId = m_pInputStream->GetCurrentGroupId();

		if(packet->iStreamId < 0)
			return true;

		stream = m_pDemuxer->GetStream(packet->iStreamId);

		if(!stream)
		{
			CLog::Log(LOGERROR, "%s - Error demux packet doesn't belong to a valid stream", __FUNCTION__);
			return false;
		}

		if(stream->source == STREAM_SOURCE_NONE)
		{
			m_SelectionStreams.Clear(STREAM_NONE, STREAM_SOURCE_DEMUX);
			m_SelectionStreams.Update(m_pInputStream, m_pDemuxer);
		}
		return true;
	}
	return false;
}

bool CDVDPlayer::IsValidStream(CCurrentStream& stream)
{
	if(stream.id < 0)
		return true; // We consider non selected as valid

	int source = STREAM_SOURCE_MASK(stream.source);
	if(source == STREAM_SOURCE_TEXT)
		return true;
	
	if(source == STREAM_SOURCE_DEMUX_SUB)
	{
		CDemuxStream* st = m_pSubtitleDemuxer->GetStream(stream.id);
		if(st == NULL || st->disabled)
			return false;

		if(st->type != stream.type)
			return false;

		return true;
	}

	if(source == STREAM_SOURCE_DEMUX)
	{
		CDemuxStream* st = m_pDemuxer->GetStream(stream.id);
		if(st == NULL || st->disabled)
			return false;

		if(st->type != stream.type)
			return false;

		if(m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
		{
			if(stream.type == STREAM_AUDIO    && st->iPhysicalId != m_dvd.iSelectedAudioStream)
				return false;
			
			if(stream.type == STREAM_SUBTITLE && st->iPhysicalId != m_dvd.iSelectedSPUStream)
				return false;
		}
		return true;
	}

	return false;
}

bool CDVDPlayer::IsBetterStream(CCurrentStream& current, CDemuxStream* stream)
{
	// Do not reopen non-video streams if we're in video-only mode
	if(m_PlayerOptions.video_only && current.type != STREAM_VIDEO)
		return false;

	if(m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
	{
		int source_type;

		source_type = STREAM_SOURCE_MASK(current.source);
		if(source_type != STREAM_SOURCE_DEMUX && source_type != STREAM_SOURCE_NONE)
			return false;

		source_type = STREAM_SOURCE_MASK(stream->source);
		if(source_type  != STREAM_SOURCE_DEMUX
		|| stream->type != current.type
		|| stream->iId  == current.id)
		return false;

		if(current.type == STREAM_AUDIO && stream->iPhysicalId == m_dvd.iSelectedAudioStream)
			return true;

		if(current.type == STREAM_SUBTITLE && stream->iPhysicalId == m_dvd.iSelectedSPUStream)
			return true;
		
		if(current.type == STREAM_VIDEO && current.id < 0)
			return true;
	}
	else
	{
		if(stream->source == current.source && stream->iId == current.id)
			return false;

		if(stream->disabled)
			return false;

		if(stream->type != current.type)
			return false;

		if(current.type == STREAM_SUBTITLE)
			return false;

		if(current.id < 0)
			return true;
	}
	return false;
}

void CDVDPlayer::UpdateApplication(double timeout)
{
	if(m_UpdateApplication != 0 && m_UpdateApplication + DVD_MSEC_TO_TIME(timeout) > CDVDClock::GetAbsoluteClock())
		return;
/*
	CDVDInputStream::IChannel* pStream = dynamic_cast<CDVDInputStream::IChannel*>(m_pInputStream);
	if(pStream)
	{
		CFileItem item(g_application.CurrentFileItem());
		if(pStream->UpdateItem(item))
		{
			g_application.CurrentFileItem() = item;
			g_infoManager.SetCurrentItem(item);
		}
	}
*/	m_UpdateApplication = CDVDClock::GetAbsoluteClock();
}

void CDVDPlayer::UpdatePlayState(double timeout)
{
	CSingleLock lock(m_StateSection);

	if(m_State.timestamp != 0 && m_State.timestamp + DVD_MSEC_TO_TIME(timeout) > CDVDClock::GetAbsoluteClock())
		return;

	if(m_CurrentVideo.dts != DVD_NOPTS_VALUE)
		m_State.dts = m_CurrentVideo.dts;
	else if(m_CurrentAudio.dts != DVD_NOPTS_VALUE)
		m_State.dts = m_CurrentAudio.dts;
	else
		m_State.dts = m_clock.GetClock();

	if(m_pDemuxer)
	{
		m_State.chapter       = m_pDemuxer->GetChapter();
		m_State.chapter_count = m_pDemuxer->GetChapterCount();
		m_pDemuxer->GetChapterName(m_State.chapter_name);

		m_State.time       = DVD_TIME_TO_MSEC(m_clock.GetClock());
		m_State.time_total = m_pDemuxer->GetStreamLength();
	}

	if(m_pInputStream)
	{
		// Override from input stream if needed
		if(m_pInputStream->IsStreamType(DVDSTREAM_TYPE_TV))
		{
//			m_State.canrecord = static_cast<CDVDInputStreamTV*>(m_pInputStream)->CanRecord();
//			m_State.recording = static_cast<CDVDInputStreamTV*>(m_pInputStream)->IsRecording();
		}

		CDVDInputStream::IDisplayTime* pDisplayTime = dynamic_cast<CDVDInputStream::IDisplayTime*>(m_pInputStream);
		if(pDisplayTime)
		{
			m_State.time       = pDisplayTime->GetTime();
			m_State.time_total = pDisplayTime->GetTotalTime();
		}

		if(m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
		{
/*			if(m_dvd.state == DVDSTATE_STILL)
			{
				m_State.time       = GetTickCount() - m_dvd.iDVDStillStartTime;
				m_State.time_total = m_dvd.iDVDStillTime;
			}

			if(!((CDVDInputStreamNavigator*)m_pInputStream)->GetNavigatorState(m_State.player_state))
				m_State.player_state = "";
*/		}
		else
			m_State.player_state = "";

		if(m_pInputStream->IsStreamType(DVDSTREAM_TYPE_TV))
		{
/*			if(((CDVDInputStreamTV*)m_pInputStream)->GetTotalTime() > 0)
			{
				m_State.time      -= ((CDVDInputStreamTV*)m_pInputStream)->GetStartTime();
				m_State.time_total = ((CDVDInputStreamTV*)m_pInputStream)->GetTotalTime();
			}
*/		}
	}

	if(m_Edl.HasCut())
	{
		m_State.time        = m_Edl.RemoveCutTime(MathUtils::rint(m_State.time));
		m_State.time_total  = m_Edl.RemoveCutTime(MathUtils::rint(m_State.time_total));
	}

	if (m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
		m_State.time_offset = DVD_MSEC_TO_TIME(m_State.time) - m_State.dts;
	else
		m_State.time_offset = 0;

	if(m_CurrentAudio.id >= 0 && m_pDemuxer)
	{
		CDemuxStream* pStream = m_pDemuxer->GetStream(m_CurrentAudio.id);
		if(pStream && pStream->type == STREAM_AUDIO)
			((CDemuxStreamAudio*)pStream)->GetStreamInfo(m_State.demux_audio);
	}
	else
		m_State.demux_audio = "";

	if(m_CurrentVideo.id >= 0 && m_pDemuxer)
	{
		CDemuxStream* pStream = m_pDemuxer->GetStream(m_CurrentVideo.id);
		if(pStream && pStream->type == STREAM_VIDEO)
			((CDemuxStreamVideo*)pStream)->GetStreamInfo(m_State.demux_video);
	}
	else
		m_State.demux_video = "";

	m_State.timestamp = CDVDClock::GetAbsoluteClock();
}
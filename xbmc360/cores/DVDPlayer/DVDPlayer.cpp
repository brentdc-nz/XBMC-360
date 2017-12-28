#include "DVDPlayer.h"
#include "DVDDemuxers\DVDDemuxFFmpeg.h" //Fixme -  need factory
#include "DVDDemuxers\DVDDemuxUtils.h"
#include "..\..\utils\log.h"
#include "DVDStreamInfo.h"
#include "..\..\Application.h"

CDVDPlayer::CDVDPlayer(IPlayerCallback& callback)
    : IPlayer(callback),
	CThread(),	
	m_dvdPlayerVideo(&m_clock/*, &m_overlayContainer*/),
    m_dvdPlayerAudio(&m_clock)
{
	m_pDemuxer = NULL;
//	m_pInputStream = NULL;
  
	m_hReadyEvent = CreateEvent(NULL, true, false, NULL);

	InitializeCriticalSection(&m_critStreamSection);

	memset(&m_dvd, 0, sizeof(DVDInfo));
	m_dvd.iFlagSentStart = 0;

	m_strFilename = "";
	m_pDemuxer = NULL;

	m_bAbortRequest = false;

	m_CurrentAudio.id = -1;
	m_CurrentAudio.dts = DVD_NOPTS_VALUE;
	m_CurrentAudio.hint.Clear();
	m_CurrentAudio.stream = NULL;

	m_CurrentVideo.id = -1;
	m_CurrentVideo.dts = DVD_NOPTS_VALUE;
	m_CurrentVideo.hint.Clear();
	m_CurrentVideo.stream = NULL;

	m_pCurrentDemuxStreamVideo = NULL;
	m_pCurrentDemuxStreamAudio = NULL;

	m_bDontSkipNextFrame = false;
}

CDVDPlayer::~CDVDPlayer()
{
	CloseFile();

	CloseHandle(m_hReadyEvent);
	DeleteCriticalSection(&m_critStreamSection);
}

bool CDVDPlayer::OpenFile(const string& strFile)
{
	try
	{
		//m_strFilename = file.m_strPath; //TODO: extract from item/file object
		m_strFilename = strFile; // TODO: Remove

		CLog::Log(LOGNOTICE, "DVDPlayer: Opening: %s", m_strFilename.c_str());

		// if playing a file close it first
		// this has to be changed so we won't have to close it.
		CloseFile();

		m_bAbortRequest = false;
		SetPlaySpeed(DVD_PLAYSPEED_NORMAL);

/*		m_dvd.state = DVDSTATE_NORMAL; // TODO Marty
		m_dvd.iSelectedSPUStream = -1;
		m_dvd.iSelectedAudioStream = -1;

		if (strFile.Find("dvd://") >= 0 ||
			strFile.CompareNoCase("d:\\video_ts\\video_ts.ifo") == 0 ||
			trFile.CompareNoCase("iso9660://video_ts/video_ts.ifo") == 0)
		{
			strcpy(m_filename, "\\Device\\Cdrom0");
		}
		else strcpy(m_filename, m_strFilename.c_str());
*/
		ResetEvent(m_hReadyEvent);
		Create();
		WaitForSingleObject(m_hReadyEvent, INFINITE);

		// if we are playing a media file with pictures, we should wait for the video output device to be initialized
		// if we don't wait, the fullscreen window will init with a picture that is 0 pixels width and high
		// we also have to wait for the player to be initialized so that we can set and access all settings when playing a dvd
		bool bProcessThreadIsAlive = true;    

		while ((m_CurrentVideo.id >= 0 || m_packetcount < 10)
			&& bProcessThreadIsAlive 
			&& !m_bAbortRequest 
			&& !m_dvdPlayerVideo.InitializedOutputDevice())
		{
			bProcessThreadIsAlive = !WaitForThreadExit(0);
			Sleep(1);
		}

		// m_bPlaying could be set to false in the meantime, which indicates an error
		if (!bProcessThreadIsAlive || m_bStop) return false;

		return true;	
	}
	catch(...)
	{
		CLog::Log(LOGERROR, __FUNCTION__" - Exception thrown on open");
		return false;
	}
}

bool CDVDPlayer::CloseFile()
{
	CLog::Log(LOGNOTICE, "CDVDPlayer::CloseFile()");

	// set the abort request so that other threads can finish up
	m_bAbortRequest = true;

	// unpause the player
	SetPlaySpeed(DVD_PLAYSPEED_NORMAL);

	// flush all buffers, and let OnExit do the rest of all the work
	// doing all the closing in OnExit requires less thread synchronisation and locking
	FlushBuffers();

	CLog::Log(LOGNOTICE, "DVDPlayer: waiting for threads to exit");

	// wait for the main thread to finish up
	// since this main thread cleans up all other resources and threads
	// we are done after the StopThread call
	StopThread();

	CLog::Log(LOGNOTICE, "DVDPlayer: finished waiting");

	return true;
}

bool CDVDPlayer::IsPlaying() const
{
	return !m_bStop;
}

bool CDVDPlayer::IsPaused() const
{
	return (m_playSpeed == DVD_PLAYSPEED_PAUSE);
}

bool CDVDPlayer::HasVideo()
{
//	if (m_pInputStream)
	{
		if ( m_CurrentVideo.id >= 0 ) return true;
		//if (/*m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD) || *\ m_CurrentVideo.id >= 0) return true;
	}
	return false;
}

bool CDVDPlayer::HasAudio()
{
	if ( m_CurrentAudio.id >= 0 ) return true;

	return false;
}

void CDVDPlayer::OnStartup()
{
	CThread::SetName("CDVDPlayer");
	m_CurrentVideo.id = -1;
	m_CurrentAudio.id = -1;
	m_packetcount = 0;

	m_messenger.Init();
}

void CDVDPlayer::Process()
{
	int video_index = -1;
	int audio_index = -1;

	CLog::Log(LOGNOTICE, "Creating Demuxer");

	m_pDemuxer = new CDVDDemuxFFmpeg;

	m_pDemuxer->Open(m_strFilename.c_str());

	// find first audio / video streams
	for (int i = 0; i < m_pDemuxer->GetNrOfStreams(); i++)
	{
		CDemuxStream* pStream = m_pDemuxer->GetStream(i);
		if (pStream->type == STREAM_AUDIO && audio_index < 0) audio_index = i;
		else if (pStream->type == STREAM_VIDEO && video_index < 0) video_index = i;
	}

	//m_dvdPlayerSubtitle.Init();
	//m_dvdPlayerSubtitle.FindSubtitles(m_filename);

	// open the streams
	if (audio_index >= 0) OpenAudioStream(audio_index);
	if (video_index >= 0) OpenVideoStream(video_index);

	// we are done initializing now, set the readyevent
	SetEvent(m_hReadyEvent);

	m_callback.OnPlayBackStarted();
	
	int iErrorCounter = 0;

	while (!m_bAbortRequest)
	{		
		// if the queues are full, no need to read more
		while (!m_bAbortRequest && (!m_dvdPlayerAudio.AcceptsData() || !m_dvdPlayerVideo.AcceptsData()))
		{
			Sleep(10);
		}

		if (!m_bAbortRequest)
		{

			if(GetPlaySpeed() != DVD_PLAYSPEED_NORMAL && GetPlaySpeed() != DVD_PLAYSPEED_PAUSE)
			{
				bool bMenu = false;//IsInMenu(); //FIXME MARTY

				// don't allow rewind in menu
				if (bMenu && GetPlaySpeed() < 0 ) SetPlaySpeed(DVD_PLAYSPEED_NORMAL);

				if (m_CurrentVideo.id >= 0 &&
					m_dvdPlayerVideo.GetCurrentPts() != DVD_NOPTS_VALUE && !bMenu)
				{
					// check how much off clock video is when ff/rw:ing
					// a problem here is that seeking isn't very accurate
					// and since the clock will be resynced after seek
					// we might actually not really be playing at the wanted
					// speed. we'd need to have some way to not resync the clock
					// after a seek to remember timing. still need to handle
					// discontinuities somehow
					// when seeking, give the player a headstart of 1 second to make sure the time it takes
					// to seek doesn't make a difference.
					__int64 iError = m_clock.GetClock() - m_dvdPlayerVideo.GetCurrentPts();

					if(GetPlaySpeed() > 0 && iError > DVD_MSEC_TO_TIME(1 * GetPlaySpeed()) )
					{
						CLog::Log(LOGDEBUG, "CDVDPlayer::Process - FF Seeking to catch up");
						SeekTime( GetTime() + GetPlaySpeed());
					}
					else if(GetPlaySpeed() < 0 && iError < DVD_MSEC_TO_TIME(1 * GetPlaySpeed()) )
					{
						SeekTime( GetTime() + GetPlaySpeed());
					}
					m_bDontSkipNextFrame = true;
				}
			}

			// handle messages send to this thread, like seek or demuxer reset requests
			HandleMessages();
//			m_bReadAgain = false;

			// read a data frame from stream.
			CDVDDemux::DemuxPacket* pPacket = m_pDemuxer->Read();

			// in a read action, the dvd navigator can do certain actions that require
			// us to read again
//			if (m_bReadAgain)
//			{
//				CDVDDemuxUtils::FreeDemuxPacket(pPacket);
//				pPacket = NULL;
//				continue;
//			}

			if (!pPacket)
			{
//				if (!m_pInputStream) break;
//				if (m_pInputStream->IsEOF()) break;
//				else if (m_dvd.state == DVDSTATE_WAIT && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
//				{
//					static_cast<CDVDInputStreamNavigator*>(m_pInputStream)->SkipWait();
//					continue;
//				}
//				else if (m_dvd.state == DVDSTATE_STILL && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD)) continue;
//				else
				{
					// keep on trying until user wants us to stop.
					iErrorCounter++;
					CLog::Log(LOGERROR, "Error reading data from demuxer");

					// maybe reseting the demuxer at this point would be a good idea, should it have failed in some way.
					// probably not a good idea, the dvdplayer can get stuck (and it does sometimes) in a loop this way.
					if (iErrorCounter > 50)
					{
						CLog::Log(LOGERROR, "got 50 read errors in a row, quiting");
						return;
						//m_pDemuxer->Reset();
						//iErrorCounter = 0;
					}
					continue;
				}
			}
	
			iErrorCounter = 0;
			m_packetcount++;

			CDemuxStream *pStream = m_pDemuxer->GetStream(pPacket->iStreamId);

			if( !pStream ) 
			{
				CLog::Log(LOGERROR, "CDVDPlayer::Process - Error demux packet doesn't belong to any stream");
				continue;
			}

			try
			{
				// for normal files, just open first stream
//				if (m_CurrentSubtitle.id < 0 && pStream->type == STREAM_SUBTITLE) OpenSubtitleStream(pStream->iId);
				if (m_CurrentAudio.id < 0    && pStream->type == STREAM_AUDIO)    OpenAudioStream(pStream->iId);
				if (m_CurrentVideo.id < 0    && pStream->type == STREAM_VIDEO)    OpenVideoStream(pStream->iId);

				// check so that none of our streams has become invalid
				if (m_CurrentAudio.id >= 0    && m_pDemuxer->GetStream(m_CurrentAudio.id) == NULL)     CloseAudioStream(false);
//				if (m_CurrentSubtitle.id >= 0 && m_pDemuxer->GetStream(m_CurrentSubtitle.id) == NULL)  CloseSubtitleStream(false);
				if (m_CurrentVideo.id >= 0    && m_pDemuxer->GetStream(m_CurrentVideo.id) == NULL)     CloseVideoStream(false);
			}
			catch (...)
			{
				CLog::Log(LOGERROR, __FUNCTION__" - Exception thrown when attempting to open stream");
				break;
			}

			LockStreams();

			try
			{
				if (pPacket->iStreamId == m_CurrentAudio.id && pStream->type == STREAM_AUDIO)
				{
					ProcessAudioData(pStream, pPacket);
				}
				else if (pPacket->iStreamId == m_CurrentVideo.id && pStream->type == STREAM_VIDEO)
				{
					ProcessVideoData(pStream, pPacket);
				}
 /*				else if (pPacket->iStreamId == m_CurrentSubtitle.id && pStream->type == STREAM_SUBTITLE)
				{
					ProcessSubData(pStream, pPacket);
  				}
*/				else CDVDDemuxUtils::FreeDemuxPacket(pPacket); // free it since we won't do anything with it
			}
			catch(...)
			{
				CLog::Log(LOGERROR, __FUNCTION__" - Exception thrown when processing demux packet");
				break;
			}

			UnlockStreams();
		}
	}
}

void CDVDPlayer::OnExit()
{
	try
	{
		CLog::Log(LOGNOTICE, "CDVDPlayer::OnExit()");

		// close each stream
		if (!m_bAbortRequest) CLog::Log(LOGNOTICE, "DVDPlayer: eof, waiting for queues to empty");
		if (m_CurrentAudio.id >= 0)
		{
			CLog::Log(LOGNOTICE, "DVDPlayer: closing audio stream");
			CloseAudioStream(!m_bAbortRequest);
		}
		if (m_CurrentVideo.id >= 0)
		{
			CLog::Log(LOGNOTICE, "DVDPlayer: closing video stream");
			CloseVideoStream(!m_bAbortRequest);
		}

		// destroy the demuxer
		if (m_pDemuxer)
		{
			CLog::Log(LOGNOTICE, "CDVDPlayer::OnExit() deleting demuxer");
			m_pDemuxer->Dispose();
			delete m_pDemuxer;
		}
		m_pDemuxer = NULL;

		// destroy the inputstream
/*		if (m_pInputStream)
		{
			CLog::Log(LOGNOTICE, "CDVDPlayer::OnExit() deleting input stream");
			delete m_pInputStream;
		}
		m_pInputStream = NULL;
*/
		// close subtitle stuff
		CLog::Log(LOGNOTICE, "CDVDPlayer::OnExit() deiniting subtitle handler");
//		m_dvdPlayerSubtitle.DeInit();
  
		// if we didn't stop playing, advance to the next item in xbmc's playlist
		if (!m_bAbortRequest) m_callback.OnPlayBackEnded();

		m_messenger.End();
	}
	catch (...)
	{
		CLog::Log(LOGERROR, __FUNCTION__" - Exception thrown when trying to close down player, memory leak will follow");
//		m_pInputStream = NULL;
		m_pDemuxer = NULL;   
	}

	m_bStop = true;

	if (m_bAbortRequest)
		m_callback.OnPlayBackStopped();

	// set event to inform openfile something went wrong in case openfile is still waiting for this event
	SetEvent(m_hReadyEvent);
}

void CDVDPlayer::HandleMessages()
{
	CDVDMsg* pMsg;

	MsgQueueReturnCode ret = m_messenger.Get(&pMsg, 0);

	while (ret == MSGQ_OK)
	{
		LockStreams();

		try
		{
			if (pMsg->IsType(CDVDMsg::PLAYER_SEEK))
			{
				CDVDMsgPlayerSeek* pMsgPlayerSeek = (CDVDMsgPlayerSeek*)pMsg;
        
				if (/*m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD)*/0)
				{
					// We can't seek in a menu
/*					if (!IsInMenu())
					{
						// need to get the seek based on file positition working in CDVDInputStreamNavigator
						// so that demuxers can control the stream (seeking in this case)
						// for now use time based seeking
						CLog::Log(LOGDEBUG, "CDVDInputStreamNavigator seek to: %d", pMsgPlayerSeek->GetTime());
						if (((CDVDInputStreamNavigator*)m_pInputStream)->Seek(pMsgPlayerSeek->GetTime()))
						{
							CLog::Log(LOGDEBUG, "CDVDInputStreamNavigator seek to: %d, succes", pMsgPlayerSeek->GetTime());
							//FlushBuffers(); buffers will be flushed by a hop from dvdnavigator
						}
						else CLog::Log(LOGWARNING, "error while seeking");
					}
					else CLog::Log(LOGWARNING, "can't seek in a menu");
*/				}
				else
				{
					CLog::Log(LOGDEBUG, "demuxer seek to: %d", pMsgPlayerSeek->GetTime());
					if (m_pDemuxer && m_pDemuxer->Seek(pMsgPlayerSeek->GetTime()))
					{
						CLog::Log(LOGDEBUG, "demuxer seek to: %d, succes", pMsgPlayerSeek->GetTime());
						FlushBuffers();
					}
					else CLog::Log(LOGWARNING, "error while seeking");
				}
				// set flag to indicate we have finished a seeking request
//				g_infoManager.m_performingSeek = false;
			}

		}
		catch (...)
		{
			CLog::Log(LOGERROR, __FUNCTION__" - Exception thrown when handling message");
		}
    
		UnlockStreams();
    
		pMsg->Release();
		ret = m_messenger.Get(&pMsg, 0);
	}
}

void CDVDPlayer::SetPlaySpeed(int speed)
{
	m_playSpeed = speed;

	// the clock needs to be paused or unpaused by seperate calls
	// audio and video part do not
	if (speed == DVD_PLAYSPEED_NORMAL) m_clock.Resume();
	else if (speed == DVD_PLAYSPEED_PAUSE) m_clock.Pause();
	else m_clock.SetSpeed(speed); // XXX

	// if playspeed is different then DVD_PLAYSPEED_NORMAL or DVD_PLAYSPEED_PAUSE
	// audioplayer, stops outputing audio to audiorendere, but still tries to
	// sleep an correct amount for each packet
	// videoplayer just plays faster after the clock speed has been increased
	// 1. disable audio
	// 2. skip frames and adjust their pts or the clock
	m_dvdPlayerAudio.SetSpeed(speed);
	m_dvdPlayerVideo.SetSpeed(speed);
}

void CDVDPlayer::Pause()
{
	int iSpeed = GetPlaySpeed();

	// return to normal speed if it was paused before, pause otherwise
	if (iSpeed == DVD_PLAYSPEED_PAUSE) SetPlaySpeed(DVD_PLAYSPEED_NORMAL);
	else SetPlaySpeed(DVD_PLAYSPEED_PAUSE);
}

bool CDVDPlayer::OnAction(const CAction &action)
{
	//TODO:

	// return false to inform the caller we didn't handle the message
	return false;
}

void CDVDPlayer::SeekTime(__int64 iTime)
{
	m_messenger.Put(new CDVDMsgPlayerSeek((int)iTime));
}

// return the time in milliseconds
__int64 CDVDPlayer::GetTime()
{
	// get timing and seeking from libdvdnav for dvd's
//	if (m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
//	{
//		return ((CDVDInputStreamNavigator*)m_pInputStream)->GetTime(); // we should take our buffers into account
//	}

	__int64 iMsecs = (m_clock.GetClock() / (DVD_TIME_BASE / 1000));
	//if (m_pDemuxer)
	//{
	//  int iMsecsStart = m_pDemuxer->GetStreamStart();
	//  if (iMsecs > iMsecsStart) iMsecs -=iMsecsStart;
	//}

	return iMsecs;
}

void CDVDPlayer::GetVideoInfo(CStdString& strVideoInfo)
{
	if( m_bStop ) return;

	string strDemuxerInfo;
	CStdString strPlayerInfo;
	if (m_CurrentVideo.id >= 0)
	{
		CDemuxStream* pStream = m_pDemuxer->GetStream(m_CurrentVideo.id);
		if (pStream && pStream->type == STREAM_VIDEO)
		{
			((CDemuxStreamVideo*)pStream)->GetStreamInfo(strDemuxerInfo);
		}
	}

	int bsize = m_dvdPlayerVideo.m_messageQueue.GetDataSize();
	if (bsize > 0) bsize = (int)(((double)m_dvdPlayerVideo.m_messageQueue.GetDataSize() / m_dvdPlayerVideo.m_messageQueue.GetMaxDataSize()) * 100);
	if (bsize > 99) bsize = 99;

	strPlayerInfo.Format("vq size: %i, cpu: %i%%", bsize, (int)(m_dvdPlayerVideo.GetRelativeUsage()*100));
	strVideoInfo.Format("D( %s ), P( %s )", strDemuxerInfo.c_str(), strPlayerInfo.c_str());
}

void CDVDPlayer::GetAudioInfo(CStdString& strAudioInfo)
{
	string strDemuxerInfo;
	CStdString strPlayerInfo;
	if (!m_bStop && m_CurrentAudio.id >= 0)
	{
		CDemuxStream* pStream = m_pDemuxer->GetStream(m_CurrentAudio.id);
		if (pStream && pStream->type == STREAM_AUDIO)
		{
			((CDemuxStreamAudio*)pStream)->GetStreamInfo(strDemuxerInfo);
		}
	}

	int bsize = m_dvdPlayerAudio.m_messageQueue.GetDataSize();
	if (bsize > 0) bsize = (int)(((double)m_dvdPlayerAudio.m_messageQueue.GetDataSize() / m_dvdPlayerAudio.m_messageQueue.GetMaxDataSize()) * 100);
	if (bsize > 99) bsize = 99;
	strPlayerInfo.Format("aq size: %i, cpu: %i%%", bsize, (int)(m_dvdPlayerAudio.GetRelativeUsage()*100));
	strAudioInfo.Format("D( %s ), P( %s )", strDemuxerInfo.c_str(), strPlayerInfo.c_str());
}

void CDVDPlayer::GetGeneralInfo(CStdString& strGeneralInfo)
{
	if (!m_bStop)
	{
		double dDelay = (double)m_dvdPlayerVideo.GetDelay() / DVD_TIME_BASE;

		__int64 apts = m_dvdPlayerAudio.GetCurrentPts();
		__int64 vpts = m_dvdPlayerVideo.GetCurrentPts();
		double dDiff = 0;

		if( apts != DVD_NOPTS_VALUE && vpts != DVD_NOPTS_VALUE )
			dDiff = (double)(apts - vpts) / DVD_TIME_BASE;    
    
		int iFramesDropped = m_dvdPlayerVideo.GetNrOfDroppedFrames();
    
		strGeneralInfo.Format("DVD Player ad:%6.3f, a/v:%6.3f, dropped:%d, cpu: %i%%", dDelay, dDiff, iFramesDropped, (int)(CThread::GetRelativeUsage()*100));
	}
}

void CDVDPlayer::SyncronizePlayers(DWORD sources)
{
	/* we need a big timeout as audio queue is about 8seconds for 2ch ac3 */
	const int timeout = 10*1000; // in milliseconds

	CDVDMsgGeneralSynchronize* message = new CDVDMsgGeneralSynchronize(timeout, sources);
	if (m_CurrentAudio.id >= 0)
	{
		message->Acquire();
		m_dvdPlayerAudio.SendMessage(message);
	}
	if (m_CurrentVideo.id >= 0)
	{
		message->Acquire();
		m_dvdPlayerVideo.SendMessage(message);
	}
	message->Release();
}

void CDVDPlayer::CheckContinuity(CDVDDemux::DemuxPacket* pPacket, unsigned int source)
{  
	/* special case for looping stillframes THX test discs*/
	if (source == DVDPLAYER_VIDEO)
	{
		__int64 missing = m_CurrentAudio.dts - m_CurrentVideo.dts;
		if( missing > DVD_MSEC_TO_TIME(100) )
		{
			if( m_CurrentVideo.dts == pPacket->dts )
			{
				CLog::Log(LOGDEBUG, "CDVDPlayer::CheckContinuity - Detected looping stillframe");
				SyncronizePlayers(SYNCSOURCE_VIDEO);
				return;
			}
		}
	}

	/* stream wrap back */
	if (pPacket->dts < min(m_CurrentAudio.dts, m_CurrentVideo.dts))
	{
		/* if video player is rendering a stillframe, we need to make sure */
		/* audio has finished processing it's data otherwise it will be */
		/* displayed too early */
 
		if (m_dvdPlayerVideo.IsStalled() && m_CurrentVideo.dts != DVD_NOPTS_VALUE)
			SyncronizePlayers(SYNCSOURCE_VIDEO);

		if (m_dvdPlayerAudio.IsStalled() && m_CurrentAudio.dts != DVD_NOPTS_VALUE)
			SyncronizePlayers(SYNCSOURCE_AUDIO);

		m_dvd.iFlagSentStart  = 0;
	}

	/* stream jump forward */
	if (pPacket->dts > max(m_CurrentAudio.dts, m_CurrentVideo.dts) + DVD_MSEC_TO_TIME(1000))
	{
		/* normally don't need to sync players since video player will keep playing at normal fps */
		/* after a discontinuity */
		//SyncronizePlayers(dts, pts, MSGWAIT_ALL);
		m_dvd.iFlagSentStart  = 0;
	}
}

void CDVDPlayer::ProcessAudioData(CDemuxStream* pStream, CDVDDemux::DemuxPacket* pPacket)
{
	if (m_CurrentAudio.stream != (void*)pStream)
	{
		/* check so that dmuxer hints or extra data hasn't changed */
		/* if they have, reopen stream */

		if (m_CurrentAudio.hint != CDVDStreamInfo(*pStream, true))
		{ 
			// we don't actually have to close audiostream here first, as 
			// we could send it as a stream message. only problem 
			// is how to notify player if a stream change failed.
			CloseAudioStream( true );
			OpenAudioStream( pPacket->iStreamId );
		}
		m_CurrentAudio.stream = (void*)pStream;
	}
  
	CheckContinuity(pPacket, DVDPLAYER_AUDIO);
	m_CurrentAudio.dts = pPacket->dts;

	//If this is the first packet after a discontinuity, send it as a resync
	if (!(m_dvd.iFlagSentStart & DVDPLAYER_AUDIO))
	{
		m_dvd.iFlagSentStart |= DVDPLAYER_AUDIO;    
		m_dvdPlayerAudio.SendMessage(new CDVDMsgGeneralSetClock(pPacket->pts, pPacket->dts));
	}

	if (m_CurrentAudio.id >= 0)
		m_dvdPlayerAudio.SendMessage(new CDVDMsgDemuxerPacket(pPacket, pPacket->iSize));
	else
		CDVDDemuxUtils::FreeDemuxPacket(pPacket);
}

void CDVDPlayer::ProcessVideoData(CDemuxStream* pStream, CDVDDemux::DemuxPacket* pPacket)
{
	if (m_CurrentVideo.stream != (void*)pStream)
	{
		/* check so that dmuxer hints or extra data hasn't changed */
		/* if they have reopen stream */
		if (m_CurrentVideo.hint != CDVDStreamInfo(*pStream, true))
		{
			CloseVideoStream(true);
			OpenVideoStream(pPacket->iStreamId);
		}
		m_CurrentVideo.stream = (void*)pStream;
	}

	if (m_bDontSkipNextFrame)
	{
		m_dvdPlayerVideo.SendMessage(new CDVDMsgVideoNoSkip());
		m_bDontSkipNextFrame = false;
	}
  
	if( pPacket->iSize != 4) //don't check the EOF_SEQUENCE of stillframes
	{
		CheckContinuity( pPacket, DVDPLAYER_VIDEO );
		m_CurrentVideo.dts = pPacket->dts;
	}

	//If this is the first packet after a discontinuity, send it as a resync
	if (!(m_dvd.iFlagSentStart & DVDPLAYER_VIDEO))
	{
		m_dvd.iFlagSentStart |= DVDPLAYER_VIDEO;
    
		if (m_CurrentAudio.id <= 0)
			m_dvdPlayerVideo.SendMessage(new CDVDMsgGeneralSetClock(pPacket->pts, pPacket->dts));
		else
			m_dvdPlayerVideo.SendMessage(new CDVDMsgGeneralResync(pPacket->pts, pPacket->dts));
	}

	if (m_CurrentVideo.id >= 0)
		m_dvdPlayerVideo.SendMessage(new CDVDMsgDemuxerPacket(pPacket, pPacket->iSize));
	else
		CDVDDemuxUtils::FreeDemuxPacket(pPacket);
}

/*
 * Opens the First valid available audio stream
 */
bool CDVDPlayer::OpenDefaultAudioStream()
{
	for (int i = 0; i < m_pDemuxer->GetNrOfStreams(); i++)
	{
		CDemuxStream* pStream = m_pDemuxer->GetStream(i);
		if (pStream->type == STREAM_AUDIO)
		{
			CDemuxStreamAudio* pStreamAudio = (CDemuxStreamAudio*)pStream;
			{
				if (OpenAudioStream(i)) return true;
			}
		}
	}
	return false;
}

bool CDVDPlayer::OpenAudioStream(int iStream)
{
	if( m_CurrentAudio.id >= 0 ) CloseAudioStream(true);

	CLog::Log(LOGNOTICE, "Opening audio stream: %i", iStream);

	if (!m_pDemuxer)
	{
		CLog::Log(LOGERROR, "No Demuxer");
		return false;
	}
  
	CDemuxStream* pStream = m_pDemuxer->GetStream(iStream);

	if (!pStream)
	{
		CLog::Log(LOGERROR, "Stream doesn't exist");
		return false;
	}

	if (pStream->disabled)
		return false;

	if (pStream->type != STREAM_AUDIO)
	{
		CLog::Log(LOGERROR, "Streamtype is not STREAM_AUDIO");
		return false;
	}

	if( m_CurrentAudio.id < 0 &&  m_CurrentVideo.id >= 0 )
	{
		// up until now we wheren't playing audio, but we did play video
		// this will change what is used to sync the dvdclock.
		// since the new audio data doesn't have to have any relation
		// to the current video data in the packet que, we have to
		// wait for it to empty

		// this happens if a new cell has audio data, but previous didn't
		// and both have video data

		SyncronizePlayers(SYNCSOURCE_AUDIO);
	}

	CDVDStreamInfo hint(*pStream, true);
//  if( m_CurrentAudio.id >= 0 )
//  {
//    CDVDStreamInfo* phint = new CDVDStreamInfo(hint);
//    m_dvdPlayerAudio.SendMessage(new CDVDMsgGeneralStreamChange(phint));
//  }
	bool success = false;
	try
	{
		 success = m_dvdPlayerAudio.OpenStream( hint );
	}
	catch (...)
	{
		CLog::Log(LOGERROR, __FUNCTION__" - Exception thrown in player");
		success = false;
	}

	if (!success)
	{
		/* mark stream as disabled, to disallaw further attempts*/
		CLog::Log(LOGWARNING, __FUNCTION__" - Unsupported stream %d. Stream disabled.", iStream);
		pStream->disabled = true;

		return false;
	}

	/* store information about stream */
	m_CurrentAudio.id = iStream;
	m_CurrentAudio.hint = hint;
	m_CurrentAudio.stream = (void*)pStream;

	m_dvdPlayerAudio.SetPriority(THREAD_PRIORITY_HIGHEST);

	/* set aspect ratio as requested by navigator for dvd's */ //FIXME MARTY
//	if( m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD) )
//		m_dvdPlayerVideo.m_messageQueue.Put(new CDVDMsgVideoSetAspect(static_cast<CDVDInputStreamNavigator*>(m_pInputStream)->GetVideoAspectRatio()));

	return true;
}

bool CDVDPlayer::OpenVideoStream(int iStream)
{
	if ( m_CurrentVideo.id >= 0) CloseVideoStream(true);

	CLog::Log(LOGNOTICE, "Opening video stream: %i", iStream);

	if (!m_pDemuxer)
	{
		CLog::Log(LOGERROR, "No Demuxer");
		return false;
	}
  
	CDemuxStream* pStream = m_pDemuxer->GetStream(iStream);

	if( !pStream )
	{
		CLog::Log(LOGERROR, "Stream doesn't exist");
		return false;
	}

	if( pStream->disabled )
		return false;

	if (pStream->type != STREAM_VIDEO)
	{
		CLog::Log(LOGERROR, "Streamtype is not STREAM_VIDEO");
		return false;
	}

	CDVDStreamInfo hint(*pStream, true);

	bool success = false;
	try
	{    
		success = m_dvdPlayerVideo.OpenStream(hint);
	}
	catch (...)
	{
		CLog::Log(LOGERROR, __FUNCTION__" - Exception thrown in player");
		success = false;
	}

	if (!success)
	{
		// mark stream as disabled, to disallaw further attempts
		CLog::Log(LOGWARNING, __FUNCTION__" - Unsupported stream %d. Stream disabled.", iStream);
		pStream->disabled = true;
		return false;
	}

	/* store information about stream */
	m_CurrentVideo.id = iStream;
	m_CurrentVideo.hint = hint;
	m_CurrentVideo.stream = (void*)pStream;

	m_dvdPlayerVideo.SetPriority(THREAD_PRIORITY_ABOVE_NORMAL);

	return true;
}

bool CDVDPlayer::CloseAudioStream(bool bWaitForBuffers)
{
	CLog::Log(LOGNOTICE, "Closing audio stream");
  
	LockStreams();

	// cannot close the stream if it does not exist
	if (m_CurrentAudio.id >= 0)
	{
		m_dvdPlayerAudio.CloseStream(bWaitForBuffers);

		m_CurrentAudio.id = -1;
	}
	UnlockStreams();
  
	return true;
}

bool CDVDPlayer::CloseVideoStream(bool bWaitForBuffers) // bWaitForBuffers currently not used
{
	CLog::Log(LOGNOTICE, "Closing video stream");

	if (m_CurrentVideo.id < 0) return false; // can't close stream if the stream does not exist

	m_dvdPlayerVideo.CloseStream(bWaitForBuffers);

	m_CurrentVideo.id = -1;

	return true;
}

void CDVDPlayer::FlushBuffers()
{
	m_dvdPlayerAudio.Flush();
	m_dvdPlayerVideo.Flush();

	m_messenger.Flush();

	m_dvd.iFlagSentStart = 0; //We will have a discontinuity here

	//m_bReadAgain = true; // XXX
	// this makes sure a new packet is read
}
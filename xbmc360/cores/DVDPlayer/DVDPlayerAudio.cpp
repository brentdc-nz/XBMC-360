#include "DVDPlayerAudio.h"
#include "DVDDemuxers\DVDDemuxUtils.h"
#include "..\..\utils\Log.h"

#include <assert.h>
#include "DVDCodecs\DVDAudioCodecFFmpeg.h" //FIXME Move to a codec factory!!

CPTSQueue::CPTSQueue()
{
	Flush();
}

void CPTSQueue::Add(__int64 pts, __int64 delay)
{
	TPTSItem item;
	item.pts = pts;
	item.timestamp = CDVDClock::GetAbsoluteClock() + delay;
	m_quePTSQueue.push(item);

	// Call function to make sure the queue 
	// doesn't grow should nobody call it
	Current();
}

void CPTSQueue::Flush()
{
	while( !m_quePTSQueue.empty() ) m_quePTSQueue.pop();
	m_currentPTSItem.timestamp = 0;
	m_currentPTSItem.pts = DVD_NOPTS_VALUE;
}

__int64 CPTSQueue::Current()
{   
	while( !m_quePTSQueue.empty() && CDVDClock::GetAbsoluteClock() >= m_quePTSQueue.front().timestamp )
	{
		m_currentPTSItem = m_quePTSQueue.front();
		m_quePTSQueue.pop();
	}

	if( m_currentPTSItem.timestamp == 0 ) return m_currentPTSItem.pts;

	return m_currentPTSItem.pts + (CDVDClock::GetAbsoluteClock() - m_currentPTSItem.timestamp);  
}

CDVDPlayerAudio::CDVDPlayerAudio(CDVDClock* pClock) : CThread()
{
	m_pClock = pClock;
	m_pAudioCodec = NULL;
	m_bInitializedOutputDevice = false;
	m_iSourceChannels = 0;
	m_audioClock = 0;

	m_currentPTSItem.pts = DVD_NOPTS_VALUE;
	m_currentPTSItem.timestamp = 0;

	SetSpeed(DVD_PLAYSPEED_NORMAL);
  
	InitializeCriticalSection(&m_critCodecSection);
	m_messageQueue.SetMaxDataSize(10 * 16 * 1024);
//	g_dvdPerformanceCounter.EnableAudioQueue(&m_packetQueue);
}

CDVDPlayerAudio::~CDVDPlayerAudio()
{
//	g_dvdPerformanceCounter.DisableAudioQueue();

	// Close the stream, and don't wait for the audio to be finished
	CloseStream(true);
	DeleteCriticalSection(&m_critCodecSection);
}

bool CDVDPlayerAudio::OpenStream( CDVDStreamInfo &hints )                                 
{
	// Should alway's be NULL!!!!, it will probably crash anyway when deleting m_pAudioCodec here.
	if (m_pAudioCodec)
	{
		CLog::Log(LOGFATAL, "CDVDPlayerAudio::OpenStream() m_pAudioCodec != NULL");
		return false;
	}

	CodecID codecID = hints.codec;

	CLog::Log(LOGNOTICE, "Finding audio codec for: %i", codecID);
	//m_pAudioCodec = CDVDFactoryCodec::CreateAudioCodec( pDemuxStream ); 
	m_pAudioCodec = new CDVDAudioCodecFFmpeg; //FIXME Marty Codec Factory needed!

	if (!m_pAudioCodec->Open(hints.codec, hints.channels, hints.samplerate))
	{
		m_pAudioCodec->Dispose();
		delete m_pAudioCodec;
		m_pAudioCodec = NULL;
		return false;
	}

	if( !m_pAudioCodec )
	{
		CLog::Log(LOGERROR, "Unsupported audio codec");
		return false;
	}

	m_codec = hints.codec;
	m_iSourceChannels = hints.channels;

	m_messageQueue.Init();

	CLog::Log(LOGNOTICE, "Creating audio thread");
	Create();

	return true;
}

void CDVDPlayerAudio::CloseStream(bool bWaitForBuffers)
{
	// Wait until buffers are empty
	if (bWaitForBuffers) m_messageQueue.WaitUntilEmpty();

	// Send abort message to the audio queue
	m_messageQueue.Abort();

	CLog::Log(LOGNOTICE, "waiting for audio thread to exit");

	// Shut down the adio_decode thread and wait for it
	StopThread(); // Will set this->m_bStop to true
	this->WaitForThreadExit(INFINITE);

	// Uninit queue
	m_messageQueue.End();

	CLog::Log(LOGNOTICE, "Deleting audio codec");
	if (m_pAudioCodec)
	{
		m_pAudioCodec->Dispose();
		delete m_pAudioCodec;
		m_pAudioCodec = NULL;
	}

	// Flush any remaining pts values
	m_ptsQueue.Flush();
}

void CDVDPlayerAudio::OnStartup()
{
	CThread::SetName("CDVDPlayerAudio");
	pAudioPacket = NULL;
	m_audioClock = 0;
	audio_pkt_data = NULL;
	audio_pkt_size = 0;

	m_Stalled = true;
  
//	g_dvdPerformanceCounter.EnableAudioDecodePerformance(ThreadHandle());
}

void CDVDPlayerAudio::Process()
{
	CLog::Log(LOGNOTICE, "running thread: CDVDPlayerAudio::Process()");

	int result;

	// Silence data
	BYTE silence[1024];
	memset(silence, 0, 1024);

	DVDAudioFrame audioframe;

	__int64 iClockDiff = 0;
	while (!m_bStop)
	{
		// Make sure player doesn't keep processing data while paused
		while (m_speed == DVD_PLAYSPEED_PAUSE && !m_messageQueue.RecievedAbortRequest()) Sleep(5);
	
		// Don't let anybody mess with our global variables
		EnterCriticalSection(&m_critCodecSection);
		result = DecodeFrame(audioframe, m_speed != DVD_PLAYSPEED_NORMAL); // blocks if no audio is available, but leaves critical section before doing so
		LeaveCriticalSection(&m_critCodecSection);

		if( result & DECODE_FLAG_ERROR ) 
		{      
			CLog::Log(LOGERROR, "CDVDPlayerAudio::Process - Decode Error. Skipping audio frame");
			continue;
		}

		if( result & DECODE_FLAG_ABORT )
		{
			CLog::Log(LOGDEBUG, "CDVDPlayerAudio::Process - Abort recieved, exiting thread");
			break;
		}

		if( result & DECODE_FLAG_DROP ) //FIXME Marty
		{
			// Frame should be dropped. Don't let audio move ahead of the current time thou
			// we need to be able to start playing at any time
			// when playing backwords, we try to keep as small buffers as possible

			// Set the time at this delay
			m_ptsQueue.Add(audioframe.pts, m_dvdAudio.GetDelay());

			if (m_speed > 0)
			{
				__int64 timestamp = m_pClock->GetAbsoluteClock() + (audioframe.duration * DVD_PLAYSPEED_NORMAL) / m_speed;
				while( !m_bStop && timestamp > m_pClock->GetAbsoluteClock() ) Sleep(1);
			}
			continue;
		}

		if( audioframe.size == 0 )
			continue;

		// We have succesfully decoded an audio frame, openup the audio device if not already done
		if (!m_bInitializedOutputDevice)
			m_bInitializedOutputDevice = InitializeOutputDevice();

		// Add any packets play
		m_dvdAudio.AddPackets(audioframe.data, audioframe.size);

		// Store the delay for this pts value so we can calculate the current playing
		m_ptsQueue.Add(audioframe.pts, m_dvdAudio.GetDelay() - audioframe.duration);
		
		// if we where asked to resync on this packet, do so here
		if( result & DECODE_FLAG_RESYNC )
		{      
			m_pClock->Discontinuity(CLOCK_DISC_NORMAL, audioframe.pts, m_dvdAudio.GetDelay() - audioframe.duration);
			CLog::Log(LOGDEBUG, "CDVDPlayerAudio:: Resync - clock:%I64d, delay:%I64d", audioframe.pts, m_dvdAudio.GetDelay() - audioframe.duration);
		}

		if( m_ptsQueue.Current() == DVD_NOPTS_VALUE )
			continue;

#if 1//def USEOLDSYNC
		// Clock should be calculated after packets have been added as m_audioClock points to the 
		// time after they have been played

		const __int64 iCurrDiff = (m_audioClock - m_dvdAudio.GetDelay()) - m_pClock->GetClock();
		const __int64 iAvDiff = (iClockDiff + iCurrDiff)/2;

		// Check for discontinuity in the stream, use a moving average to
		// eliminate highfreq fluctuations of large packet sizes
		if( abs(iAvDiff) > 5000 ) // sync clock if average diff is bigger than 5 msec 
		{
			// Wait untill only the new audio frame wich triggered the discontinuity is left
			// then set disc state
			while (!m_bStop && (unsigned int)m_dvdAudio.GetBytesInBuffer() > audioframe.size ) Sleep(5);

			m_pClock->Discontinuity(CLOCK_DISC_NORMAL, m_audioClock - m_dvdAudio.GetDelay());
			CLog::Log(LOGDEBUG, "CDVDPlayer:: Detected Audio Discontinuity, syncing clock. diff was: %I64d, %I64d, av: %I64d", iClockDiff, iCurrDiff, iAvDiff);
			iClockDiff = 0;
		}
		else
		{      
			// Do gradual adjustments (not working yet)
			//m_pClock->AdjustSpeedToMatch(iClock + iAvDiff);
			iClockDiff = iCurrDiff;
		}
#endif
	}
}

void CDVDPlayerAudio::OnExit()
{
	//g_dvdPerformanceCounter.DisableAudioDecodePerformance();
  
	// destroy audio device
	CLog::Log(LOGNOTICE, "Closing audio device");
	m_dvdAudio.Destroy();
	m_bInitializedOutputDevice = false;

	CLog::Log(LOGNOTICE, "thread end: CDVDPlayerAudio::OnExit()");
}

// decode one audio frame and returns its uncompressed size
int CDVDPlayerAudio::DecodeFrame(DVDAudioFrame &audioframe, bool bDropPacket)
{
	CDVDDemux::DemuxPacket* pPacket = pAudioPacket;
	int n=48000*2*16/8, len;

	//Store amount left at this point, and what last pts was
	unsigned __int64 first_pkt_pts = 0;
	int first_pkt_size = 0; 
	int first_pkt_used = 0;
	int result = 0;

	// make sure the sent frame is clean
	memset(&audioframe, 0, sizeof(DVDAudioFrame));

	if (pPacket)
	{
		first_pkt_pts = pPacket->pts;
		first_pkt_size = pPacket->iSize;
		first_pkt_used = first_pkt_size - audio_pkt_size;
	}
 
	for (;;)
	{
		/* NOTE: the audio packet can contain several frames */
		while (audio_pkt_size > 0)
		{
			len = m_pAudioCodec->Decode(audio_pkt_data, audio_pkt_size);
			if (len < 0)
			{
				/* if error, we skip the frame */
				audio_pkt_size=0;
				m_pAudioCodec->Reset();
				break;
			}

			// fix for fucked up decoders //FIXME Marty
			if( len > audio_pkt_size )
			{        
				CLog::Log(LOGERROR, "CDVDPlayerAudio:DecodeFrame - Codec tried to consume more data than available. Potential memory corruption");        
				audio_pkt_size=0;
				m_pAudioCodec->Reset();
				assert(0);
			}

			// get decoded data and the size of it
			audioframe.size = m_pAudioCodec->GetData(&audioframe.data);

			audio_pkt_data += len;
			audio_pkt_size -= len;

			if (audioframe.size <= 0) continue;

			audioframe.pts = m_audioClock;

			// compute duration.
			n = m_pAudioCodec->GetChannels() * m_pAudioCodec->GetBitsPerSample() / 8 * m_pAudioCodec->GetSampleRate();
			if (n > 0)
			{
				// safety check, if channels == 0, n will result in 0, and that will result in a nice devide exception
				audioframe.duration = (unsigned int)(((__int64)audioframe.size * DVD_TIME_BASE) / n);

				// increase audioclock to after the packet
				m_audioClock += audioframe.duration;
			}

			//If we are asked to drop this packet, return a size of zero. then it won't be played
			//we currently still decode the audio.. this is needed since we still need to know it's 
			//duration to make sure clock is updated correctly.
			if( bDropPacket )
			{
				result |= DECODE_FLAG_DROP;
			}
			return result;
		}

		// free the current packet
		if (pPacket)
		{
			CDVDDemuxUtils::FreeDemuxPacket(pPacket); //Marty FIXME
			pPacket = NULL;
			pAudioPacket = NULL;
		}

		if (m_messageQueue.RecievedAbortRequest()) return DECODE_FLAG_ABORT;

		// read next packet and return -1 on error
		LeaveCriticalSection(&m_critCodecSection); //Leave here as this might stall a while

		CDVDMsg* pMsg;
		MsgQueueReturnCode ret = m_messageQueue.Get(&pMsg, INFINITE);

		EnterCriticalSection(&m_critCodecSection);

		if (ret == MSGQ_TIMEOUT) 
		{
			m_Stalled = true;
			continue;
		}

		if (MSGQ_IS_ERROR(ret) || ret == MSGQ_ABORT) return DECODE_FLAG_ABORT;

		if (pMsg->IsType(CDVDMsg::DEMUXER_PACKET))
		{
			CDVDMsgDemuxerPacket* pMsgDemuxerPacket = (CDVDMsgDemuxerPacket*)pMsg;
			pPacket = pMsgDemuxerPacket->GetPacket();
			pMsgDemuxerPacket->m_pPacket = NULL; // XXX, test
			pAudioPacket = pPacket;
			audio_pkt_data = pPacket->pData;
			audio_pkt_size = pPacket->iSize;
			m_Stalled = false;
		}
		else if (pMsg->IsType(CDVDMsg::GENERAL_STREAMCHANGE))
		{
			CDVDMsgGeneralStreamChange* pMsgStreamChange = (CDVDMsgGeneralStreamChange*)pMsg;
			CDVDStreamInfo* hints = pMsgStreamChange->GetStreamInfo();

			/* recieved a stream change, reopen codec. */
			/* we should really not do this untill first packet arrives, to have a probe buffer */      

			/* try to open decoder, if none is found keep consuming packets */
//			OpenDecoder( *hints );

			pMsg->Release();
			continue;
		}
		else if (pMsg->IsType(CDVDMsg::GENERAL_SYNCHRONIZE))
		{
			CDVDMsgGeneralSynchronize* pMsgGeneralSynchronize = (CDVDMsgGeneralSynchronize*)pMsg;
      
			CLog::Log(LOGDEBUG, "CDVDPlayerAudio - CDVDMsg::GENERAL_SYNCHRONIZE");

			pMsgGeneralSynchronize->Wait( &m_bStop, SYNCSOURCE_AUDIO );
      
			pMsg->Release();
			continue;
		} 
		else if (pMsg->IsType(CDVDMsg::GENERAL_SET_CLOCK))
		{ 
			//player asked us to set dvdclock on this
			CDVDMsgGeneralSetClock* pMsgGeneralSetClock = (CDVDMsgGeneralSetClock*)pMsg;
			result |= DECODE_FLAG_RESYNC;
      
			if( pMsgGeneralSetClock->GetDts() != DVD_NOPTS_VALUE )
				m_audioClock = pMsgGeneralSetClock->GetDts();

			pMsg->Release();
			continue;
		} 
		else if (pMsg->IsType(CDVDMsg::GENERAL_RESYNC))
		{ 
			//player asked us to set internal clock
			CDVDMsgGeneralResync* pMsgGeneralResync = (CDVDMsgGeneralResync*)pMsg;                  

			if( pMsgGeneralResync->GetDts() != DVD_NOPTS_VALUE )
				m_audioClock = pMsgGeneralResync->GetDts();
      
			pMsg->Release();
				continue;
		}
   
		if (pMsg->IsType(CDVDMsg::DEMUXER_PACKET))
		{
			if (pPacket->pts != DVD_NOPTS_VALUE) // CDVDMsg::DEMUXER_PACKET, pPacket is already set above
			{
				if (first_pkt_size == 0) 
				{ 
					//first package
					m_audioClock = pPacket->pts;        
				}
				else if (first_pkt_pts > pPacket->pts)
				{ 
					//okey first packet in this continous stream, make sure we use the time here        
					m_audioClock = pPacket->pts;        
				}
				else if((unsigned __int64)m_audioClock < pPacket->pts || (unsigned __int64)m_audioClock > pPacket->pts)
				{
					//crap, moved outsided correct pts
					//Use pts from current packet, untill we find a better value for it.
					//Should be ok after a couple of frames, as soon as it starts clean on a packet
					m_audioClock = pPacket->pts;
				}
				else if(first_pkt_size == first_pkt_used)
				{ 
					//Nice starting up freshly on the start of a packet, use pts from it
					m_audioClock = pPacket->pts;
				}
			}
		}
		pMsg->Release();
	}
}

void CDVDPlayerAudio::SetSpeed(int speed)
{ 
	m_speed = speed;
  
	if (m_speed == DVD_PLAYSPEED_PAUSE)
	{
		m_ptsQueue.Flush();
		m_dvdAudio.Pause();
	}
	else 
		m_dvdAudio.Resume();
}

void CDVDPlayerAudio::Flush()
{
	m_messageQueue.Flush();
	m_dvdAudio.Flush();
	m_ptsQueue.Flush();

	if (m_pAudioCodec)
	{
		EnterCriticalSection(&m_critCodecSection);
		
		audio_pkt_size = 0;
		audio_pkt_data = NULL;

		if( pAudioPacket )
		{
			CDVDDemuxUtils::FreeDemuxPacket(pAudioPacket);
			pAudioPacket = NULL;
		}

		m_pAudioCodec->Reset();

		LeaveCriticalSection(&m_critCodecSection);
	}
}

bool CDVDPlayerAudio::InitializeOutputDevice()
{
	int iChannels = m_pAudioCodec->GetChannels();
	int iSampleRate = m_pAudioCodec->GetSampleRate();
	int iBitsPerSample = m_pAudioCodec->GetBitsPerSample();
	//bool bPasstrough = m_pAudioCodec->NeedPasstrough(); //Marty

	if (iChannels == 0 || iSampleRate == 0 || iBitsPerSample == 0)
	{
		CLog::Log(LOGERROR, "Unable to create audio device, (iChannels == 0 || iSampleRate == 0 || iBitsPerSample == 0)");
		return false;
	}

	CLog::Log(LOGNOTICE, "Creating audio device with codec id: %i, channels: %i, sample rate: %i", m_codec, iChannels, iSampleRate);
	if (m_dvdAudio.Create(iChannels, iSampleRate, iBitsPerSample, /*bPasstrough*/false)) // always 16 bit with ffmpeg ? //Marty Passthrough needed?
		return true;

	CLog::Log(LOGERROR, "Failed Creating audio device with codec id: %i, channels: %i, sample rate: %i", m_codec, iChannels, iSampleRate);
	return false;
}
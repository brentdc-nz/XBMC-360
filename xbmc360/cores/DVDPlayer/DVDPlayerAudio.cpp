#include "DVDPlayerAudio.h"
#include "utils\Log.h"
#include "DVDPerformanceCounter.h"
#include "DVDCodecs\DVDFactoryCodec.h"
#include "DVDPlayer.h"
#include "utils\TimeUtils.h"
#include <sstream>
#include <iomanip>
#include "utils\MathUtils.h"

using namespace std;

CPTSOutputQueue::CPTSOutputQueue()
{
	Flush();
}

void CPTSOutputQueue::Add(double pts, double delay, double duration)
{
	CSingleLock lock(m_sync);

	TPTSItem item;
	item.pts = pts;
	item.timestamp = CDVDClock::GetAbsoluteClock() + delay;
	item.duration = duration;

	// First one is applied directly
	if(m_queue.empty() && m_current.pts == DVD_NOPTS_VALUE)
		m_current = item;
	else
		m_queue.push(item);

	// Call function to make sure the queue
	// doesn't grow should nobody call it
	Current();
}
void CPTSOutputQueue::Flush()
{
	CSingleLock lock(m_sync);

	while(!m_queue.empty()) m_queue.pop();
	m_current.pts = DVD_NOPTS_VALUE;
	m_current.timestamp = 0.0;
	m_current.duration = 0.0;
}

double CPTSOutputQueue::Current()
{
	CSingleLock lock(m_sync);

	if(!m_queue.empty() && m_current.pts == DVD_NOPTS_VALUE)
	{
		m_current = m_queue.front();
		m_queue.pop();
	}

	while(!m_queue.empty() && CDVDClock::GetAbsoluteClock() >= m_queue.front().timestamp)
	{
		m_current = m_queue.front();
		m_queue.pop();
	}

	if(m_current.timestamp == 0) return m_current.pts;

	return m_current.pts + min(m_current.duration, (CDVDClock::GetAbsoluteClock() - m_current.timestamp));
}

void CPTSInputQueue::Add(__int64 bytes, double pts)
{
	CSingleLock lock(m_sync);

	m_list.insert(m_list.begin(), make_pair(bytes, pts));
}

void CPTSInputQueue::Flush()
{
	CSingleLock lock(m_sync);

	m_list.clear();
}
double CPTSInputQueue::Get(__int64 bytes, bool consume)
{
	CSingleLock lock(m_sync);

	IT it = m_list.begin();
	for(; it != m_list.end(); it++)
	{
		if(bytes <= it->first)
		{
			double pts = it->second;
			if(consume)
			{
				it->second = DVD_NOPTS_VALUE;
				m_list.erase(++it, m_list.end());
			}
			return pts;
		}
		bytes -= it->first;
	}
	return DVD_NOPTS_VALUE;
}

class CDVDMsgAudioCodecChange : public CDVDMsg
{
public:
	CDVDMsgAudioCodecChange(const CDVDStreamInfo &hints, CDVDAudioCodec* codec)
	: CDVDMsg(GENERAL_STREAMCHANGE)
	, m_codec(codec)
	, m_hints(hints)
	{
	}
	~CDVDMsgAudioCodecChange()
	{
		delete m_codec;
	}
	CDVDAudioCodec* m_codec;
	CDVDStreamInfo  m_hints;
};

CDVDPlayerAudio::CDVDPlayerAudio(CDVDClock* pClock, CDVDMessageQueue& parent)
: CThread()
, m_messageQueue("audio")
, m_messageParent(parent)
, m_dvdAudio((bool&)m_bStop)
{
	m_pClock = pClock;
	m_pAudioCodec = NULL;
	m_audioClock = 0;
	m_droptime = 0;
	m_speed = DVD_PLAYSPEED_NORMAL;
	m_stalled = true;
	m_started = false;
	m_duration = 0.0;

	m_freq = CurrentHostFrequency();
#ifdef _XBOX
	m_messageQueue.SetMaxDataSize(30 * 16 * 1024);
#else
	m_messageQueue.SetMaxDataSize(6 * 1024 * 1024);
#endif
	m_messageQueue.SetMaxTimeSize(8.0);
	g_dvdPerformanceCounter.EnableAudioQueue(&m_messageQueue);
}

CDVDPlayerAudio::~CDVDPlayerAudio()
{
	StopThread();
	g_dvdPerformanceCounter.DisableAudioQueue();
}

bool CDVDPlayerAudio::OpenStream( CDVDStreamInfo &hints )
{
	CLog::Log(LOGNOTICE, "Finding audio codec for: %i", hints.codec);

	CDVDAudioCodec* codec = CDVDFactoryCodec::CreateAudioCodec(hints);
	if(!codec)
	{
		CLog::Log(LOGERROR, "Unsupported audio codec");
		return false;
	}

	if(m_messageQueue.IsInited())
		m_messageQueue.Put(new CDVDMsgAudioCodecChange(hints, codec), 0);
	else
	{
		OpenStream(hints, codec);
		m_messageQueue.Init();
		CLog::Log(LOGNOTICE, "Creating audio thread");
		Create();
	}
	return true;
}

void CDVDPlayerAudio::OpenStream(CDVDStreamInfo &hints, CDVDAudioCodec* codec)
{
	SAFE_DELETE(m_pAudioCodec);
	m_pAudioCodec = codec;

	// Store our stream hints
	m_streaminfo = hints;

	// Update codec information from what codec gave ut
	m_streaminfo.channels = m_pAudioCodec->GetChannels();
	m_streaminfo.samplerate = m_pAudioCodec->GetSampleRate();

	m_droptime = 0;
	m_audioClock = 0;
	m_stalled = m_messageQueue.GetPacketCount(CDVDMsg::DEMUXER_PACKET) == 0;
	m_started = false;

	m_error = 0;
	m_errorbuff = 0;
	m_errorcount = 0;
	m_syncclock = true;
	m_errortime = CurrentHostCounter();
}

void CDVDPlayerAudio::CloseStream(bool bWaitForBuffers)
{
	// Wait until buffers are empty
	if(bWaitForBuffers && m_speed > 0) m_messageQueue.WaitUntilEmpty();

	// Send abort message to the audio queue
	m_messageQueue.Abort();

	CLog::Log(LOGNOTICE, "Waiting for audio thread to exit");

	// Shut down the adio_decode thread and wait for it
	StopThread(); // Will set this->m_bStop to true

	// Destroy audio device
	CLog::Log(LOGNOTICE, "Closing audio device");
	if(bWaitForBuffers && m_speed > 0)
	{
		m_bStop = false;
		m_dvdAudio.Drain();
		m_bStop = true;
	}

	m_dvdAudio.Destroy();

	// Uninit queue
	m_messageQueue.End();

	CLog::Log(LOGNOTICE, "Deleting audio codec");
	if(m_pAudioCodec)
	{
		m_pAudioCodec->Dispose();
		delete m_pAudioCodec;
		m_pAudioCodec = NULL;
	}

	// Flush any remaining pts values
	m_ptsOutput.Flush();
}

void CDVDPlayerAudio::SetSpeed(int speed)
{
	if(m_messageQueue.IsInited())
		m_messageQueue.Put(new CDVDMsgInt(CDVDMsg::PLAYER_SETSPEED, speed), 1);
	else
		m_speed = speed;
}

void CDVDPlayerAudio::Flush()
{
	m_messageQueue.Flush();
	m_messageQueue.Put(new CDVDMsg(CDVDMsg::GENERAL_FLUSH), 1);
}

void CDVDPlayerAudio::OnStartup()
{
	CThread::SetName("CDVDPlayerAudio");

	m_decode.msg = NULL;
	m_decode.Release();

	g_dvdPerformanceCounter.EnableAudioDecodePerformance(ThreadHandle());
}

void CDVDPlayerAudio::Process()
{
	CLog::Log(LOGNOTICE, "running thread: CDVDPlayerAudio::Process()");

	int result;
	bool packetadded(false);

	DVDAudioFrame audioframe;
	m_audioStats.Start();

	while(!m_bStop)
	{
		// Don't let anybody mess with our global variables
		result = DecodeFrame(audioframe, m_speed > DVD_PLAYSPEED_NORMAL || m_speed < 0); // blocks if no audio is available, but leaves critical section before doing so

		if(result & DECODE_FLAG_ERROR)
		{
			CLog::Log(LOGDEBUG, "CDVDPlayerAudio::Process - Decode Error");
			continue;
		}

		if(result & DECODE_FLAG_TIMEOUT)
		{
			m_stalled = true;

			// Flush as the audio output may keep looping if we don't
			if(m_speed == DVD_PLAYSPEED_NORMAL)
				m_dvdAudio.Flush();

			continue;
		}

		if(result & DECODE_FLAG_ABORT)
		{
			CLog::Log(LOGDEBUG, "CDVDPlayerAudio::Process - Abort received, exiting thread");
			break;
		}

#ifdef PROFILE // During profiling we just drop all packets, after having decoded
		m_pClock->Discontinuity(CLOCK_DISC_NORMAL, audioframe.pts, 0);
		continue;
#endif
		if(audioframe.size == 0)
			continue;
	
		packetadded = true;

		// We have succesfully decoded an audio frame, setup renderer to match
		if(!m_dvdAudio.IsValidFormat(audioframe))
		{
			m_dvdAudio.Destroy();
			if(!m_dvdAudio.Create(audioframe, m_streaminfo.codec))
				CLog::Log(LOGERROR, "%s - failed to create audio renderer", __FUNCTION__);
		}

		if(result & DECODE_FLAG_DROP)
		{
			// Frame should be dropped. Don't let audio move ahead of the current time thou
			// we need to be able to start playing at any time
			// when playing backwords, we try to keep as small buffers as possible

			if(m_droptime == 0.0)
				m_droptime = m_pClock->GetAbsoluteClock();
		
			if(m_speed > 0)
				m_droptime += audioframe.duration * DVD_PLAYSPEED_NORMAL / m_speed;

			while(!m_bStop && m_droptime > m_pClock->GetAbsoluteClock() ) Sleep(1);

			m_stalled = false;
		}
		else
		{
			m_droptime = 0.0;

			// Add any packets play
			packetadded = OutputPacket(audioframe);

			// We are not running until something is cached in output device
			if(m_stalled && m_dvdAudio.GetCacheTime() > 0.0)
				m_stalled = false;
		}

		// Store the delay for this pts value so we can calculate the current playing
		if(packetadded)
		{
			if(m_speed == DVD_PLAYSPEED_PAUSE)
				m_ptsOutput.Add(audioframe.pts, m_dvdAudio.GetDelay() - audioframe.duration, 0);
			else
				m_ptsOutput.Add(audioframe.pts, m_dvdAudio.GetDelay() - audioframe.duration, audioframe.duration);
		}

		// Signal to our parent that we have initialized
		if(m_started == false)
		{
			m_started = true;
			m_messageParent.Put(new CDVDMsgInt(CDVDMsg::PLAYER_STARTED, DVDPLAYER_AUDIO));
		}

		if(m_ptsOutput.Current() == DVD_NOPTS_VALUE)
			continue;

		if(m_speed != DVD_PLAYSPEED_NORMAL)
			continue;

		if(packetadded)
			HandleSyncError(audioframe.duration);
	}
}

void CDVDPlayerAudio::HandleSyncError(double duration)
{
	double clock = m_pClock->GetClock();
	double error = m_ptsOutput.Current() - clock;
	int64_t now;

	if(fabs(error) > DVD_MSEC_TO_TIME(100) || m_syncclock)
	{
		m_pClock->Discontinuity(CLOCK_DISC_NORMAL, clock+error, 0);
		if(m_speed == DVD_PLAYSPEED_NORMAL)
			CLog::Log(LOGDEBUG, "CDVDPlayerAudio:: Discontinuity - was:%f, should be:%f, error:%f", clock, clock+error, error);

		m_errorbuff = 0;
		m_errorcount = 0;
		m_error = 0;
		m_syncclock = false;
		m_errortime = CurrentHostCounter();

		return;
	}

	if(m_speed != DVD_PLAYSPEED_NORMAL)
	{
		m_errorbuff = 0;
		m_errorcount = 0;
		m_error = 0;
		m_errortime = CurrentHostCounter();
		return;
	}

	m_errorbuff += error;
	m_errorcount++;

	// Check if measured error for 1 second
	now = CurrentHostCounter();
	if((now - m_errortime) >= m_freq)
	{
		m_errortime = now;
		m_error = m_errorbuff / m_errorcount;

		m_errorbuff = 0;
		m_errorcount = 0;

		if(fabs(m_error) > DVD_MSEC_TO_TIME(10))
		{
			m_pClock->Discontinuity(CLOCK_DISC_NORMAL, clock+m_error, 0);
			if(m_speed == DVD_PLAYSPEED_NORMAL)
				CLog::Log(LOGDEBUG, "CDVDPlayerAudio:: Discontinuity - was:%f, should be:%f, error:%f", clock, clock+m_error, m_error);
		}
	}
}

// Decode one audio frame and returns its uncompressed size
int CDVDPlayerAudio::DecodeFrame(DVDAudioFrame &audioframe, bool bDropPacket)
{
	int result = 0;

	// Make sure the sent frame is clean
	memset(&audioframe, 0, sizeof(DVDAudioFrame));

	while(!m_bStop)
	{
		// NOTE: The audio packet can contain several frames
		while(!m_bStop && m_decode.size > 0)
		{
			if(!m_pAudioCodec)
				return DECODE_FLAG_ERROR;

			// The packet dts refers to the first audioframe that starts in the packet
			double dts = m_ptsInput.Get(m_decode.size + m_pAudioCodec->GetBufferSize(), true);
			if(dts != DVD_NOPTS_VALUE)
				m_audioClock = dts;

			int len = m_pAudioCodec->Decode(m_decode.data, m_decode.size);
			m_audioStats.AddSampleBytes(m_decode.size);
			if(len < 0)
			{
				// If error, we skip the packet
				CLog::Log(LOGERROR, "CDVDPlayerAudio::DecodeFrame - Decode Error. Skipping audio packet");
				m_decode.Release();
				m_pAudioCodec->Reset();
				return DECODE_FLAG_ERROR;
			}

			// Fix for fucked up decoders
			if(len > m_decode.size)
			{
				CLog::Log(LOGERROR, "CDVDPlayerAudio:DecodeFrame - Codec tried to consume more data than available. Potential memory corruption");
				m_decode.Release();
				m_pAudioCodec->Reset();
				assert(0);
			}

			m_decode.data += len;
			m_decode.size -= len;
	
			// Get decoded data and the size of it
			audioframe.size = m_pAudioCodec->GetData(&audioframe.data);
			audioframe.pts = m_audioClock;
			audioframe.channels = m_pAudioCodec->GetChannels();
			audioframe.bits_per_sample = m_pAudioCodec->GetBitsPerSample();
			audioframe.sample_rate = m_pAudioCodec->GetSampleRate();
			audioframe.passthrough = m_pAudioCodec->NeedPassthrough();

			if(audioframe.size <= 0)
				continue;

			// Compute duration
			int n = (audioframe.channels * audioframe.bits_per_sample * audioframe.sample_rate)>>3;
			if(n > 0)
			{
				// Safety check, if channels == 0, n will result in 0, and that will result in a nice devide exception
				audioframe.duration = ((double)audioframe.size * DVD_TIME_BASE) / n;

				// Increase audioclock to after the packet
				m_audioClock += audioframe.duration;
			}

			if(audioframe.duration > 0)
				m_duration = audioframe.duration;

			// If demux source want's us to not display this, continue
			if(m_decode.msg->GetPacketDrop())
				continue;

			// If we are asked to drop this packet, return a size of zero. then it won't be played
			// we currently still decode the audio.. this is needed since we still need to know it's
			// duration to make sure clock is updated correctly.
			if(bDropPacket)
				result |= DECODE_FLAG_DROP;

			return result;
		}

		// Free the current packet
		m_decode.Release();

		if(m_messageQueue.ReceivedAbortRequest()) return DECODE_FLAG_ABORT;

		CDVDMsg* pMsg;
		int priority = (m_speed == DVD_PLAYSPEED_PAUSE/* && m_started */) ? 1 : 0;
	
		int timeout;
		if(m_duration > 0)
			timeout = (int)(1000 * (m_duration / DVD_TIME_BASE + m_dvdAudio.GetCacheTime()));
		else
			timeout = 1000;

		// Read next packet and return -1 on error
		MsgQueueReturnCode ret = m_messageQueue.Get(&pMsg, timeout, priority);

		if(ret == MSGQ_TIMEOUT)
			return DECODE_FLAG_TIMEOUT;

		if(MSGQ_IS_ERROR(ret) || ret == MSGQ_ABORT)
			return DECODE_FLAG_ABORT;

		if(pMsg->IsType(CDVDMsg::DEMUXER_PACKET))
		{
			m_decode.Attach((CDVDMsgDemuxerPacket*)pMsg);
			m_ptsInput.Add( m_decode.size, m_decode.dts );
		}
		else if (pMsg->IsType(CDVDMsg::GENERAL_SYNCHRONIZE))
		{
			((CDVDMsgGeneralSynchronize*)pMsg)->Wait(&m_bStop, SYNCSOURCE_AUDIO);
			CLog::Log(LOGDEBUG, "CDVDPlayerAudio - CDVDMsg::GENERAL_SYNCHRONIZE");
		}
		else if (pMsg->IsType(CDVDMsg::GENERAL_RESYNC))
		{
			// Player asked us to set internal clock
			CDVDMsgGeneralResync* pMsgGeneralResync = (CDVDMsgGeneralResync*)pMsg;

			if(pMsgGeneralResync->m_timestamp != DVD_NOPTS_VALUE)
				m_audioClock = pMsgGeneralResync->m_timestamp;

			m_ptsOutput.Add(m_audioClock, m_dvdAudio.GetDelay(), 0);
			if (pMsgGeneralResync->m_clock)
			{
				CLog::Log(LOGDEBUG, "CDVDPlayerAudio - CDVDMsg::GENERAL_RESYNC(%f, 1)", m_audioClock);
				m_pClock->Discontinuity(CLOCK_DISC_NORMAL, m_ptsOutput.Current(), 0);
			}
			else
				CLog::Log(LOGDEBUG, "CDVDPlayerAudio - CDVDMsg::GENERAL_RESYNC(%f, 0)", m_audioClock);
		}
		else if(pMsg->IsType(CDVDMsg::GENERAL_RESET))
		{
			if(m_pAudioCodec)
				m_pAudioCodec->Reset();

			m_decode.Release();
			m_started = false;
		}
		else if(pMsg->IsType(CDVDMsg::GENERAL_FLUSH))
		{
			m_dvdAudio.Flush();
			m_ptsOutput.Flush();
			m_ptsInput.Flush();
			m_syncclock = true;
			m_stalled   = true;
			m_started   = false;

			if(m_pAudioCodec)
				m_pAudioCodec->Reset();

			m_decode.Release();
		}
		else if (pMsg->IsType(CDVDMsg::PLAYER_STARTED))
		{
			if(m_started)
				m_messageParent.Put(new CDVDMsgInt(CDVDMsg::PLAYER_STARTED, DVDPLAYER_AUDIO));
		}
		else if (pMsg->IsType(CDVDMsg::GENERAL_EOF))
		{
			CLog::Log(LOGDEBUG, "CDVDPlayerAudio - CDVDMsg::GENERAL_EOF");
			m_dvdAudio.Finish();
		}
		else if (pMsg->IsType(CDVDMsg::GENERAL_DELAY))
		{
			if(m_speed != DVD_PLAYSPEED_PAUSE)
			{
				double timeout = static_cast<CDVDMsgDouble*>(pMsg)->m_value;

				CLog::Log(LOGDEBUG, "CDVDPlayerAudio - CDVDMsg::GENERAL_DELAY(%f)", timeout);

				timeout *= (double)DVD_PLAYSPEED_NORMAL / abs(m_speed);
				timeout += CDVDClock::GetAbsoluteClock();

				while(!m_bStop && CDVDClock::GetAbsoluteClock() < timeout)
					Sleep(1);
			}
		}
		else if(pMsg->IsType(CDVDMsg::PLAYER_SETSPEED))
		{
			m_speed = static_cast<CDVDMsgInt*>(pMsg)->m_value;

			if(m_speed == DVD_PLAYSPEED_PAUSE)
			{
				m_ptsOutput.Flush();
				m_syncclock = true;
				m_dvdAudio.Pause();
			}
			else
				m_dvdAudio.Resume();
		}
		else if(pMsg->IsType(CDVDMsg::GENERAL_STREAMCHANGE))
		{
			CDVDMsgAudioCodecChange* msg(static_cast<CDVDMsgAudioCodecChange*>(pMsg));
			OpenStream(msg->m_hints, msg->m_codec);
			msg->m_codec = NULL;
		}

		pMsg->Release();
	}
	return 0;
}

bool CDVDPlayerAudio::OutputPacket(DVDAudioFrame &audioframe)
{
	m_dvdAudio.AddPackets(audioframe);
	return true;
}

void CDVDPlayerAudio::OnExit()
{
	g_dvdPerformanceCounter.DisableAudioDecodePerformance();

	CLog::Log(LOGNOTICE, "thread end: CDVDPlayerAudio::OnExit()");
}

double round(double x)
{
    return (x > 0) ? floor(x + 0.5) : ceil(x - 0.5);
}

string CDVDPlayerAudio::GetPlayerInfo()
{
	std::ostringstream strTmp;
	strTmp << "aq:"     << setw(2) << min(99,m_messageQueue.GetLevel() + MathUtils::round_int(100.0/8.0*m_dvdAudio.GetCacheTime())) << "%";
	strTmp << ", kB/s:" << fixed << setprecision(2) << (double)GetAudioBitrate() / 1024.0;
	return strTmp.str();
}

int CDVDPlayerAudio::GetAudioBitrate()
{
	return (int)m_audioStats.GetBitrate();
}
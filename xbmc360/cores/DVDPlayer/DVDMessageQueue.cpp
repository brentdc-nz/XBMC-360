#include "DVDMessageQueue.h"
#include "DVDClock.h"
#include "utils\Log.h"

CDVDMessageQueue::CDVDMessageQueue(const std::string &owner)
{
	m_owner = owner;
	m_iDataSize     = 0;
	m_bAbortRequest = false;
	m_bInitialized  = false;
	m_bCaching      = false;
	m_bEmptied      = true;
  
	m_TimeBack      = DVD_NOPTS_VALUE;
	m_TimeFront     = DVD_NOPTS_VALUE;
	m_TimeSize      = 1.0 / 4.0; /* 4 seconds */
	m_hEvent = CreateEvent(NULL, true, false, NULL);
}

CDVDMessageQueue::~CDVDMessageQueue()
{
	// Remove all remaining messages
	Flush();
  
	CloseHandle(m_hEvent);
}

void CDVDMessageQueue::Init()
{
	m_iDataSize     = 0;
	m_bAbortRequest = false;
	m_bEmptied      = true;
	m_bInitialized  = true;
	m_TimeBack      = DVD_NOPTS_VALUE;
	m_TimeFront     = DVD_NOPTS_VALUE;
}

void CDVDMessageQueue::Flush(CDVDMsg::Message type)
{
	CSingleLock lock(m_section);

	for(SList::iterator it = m_list.begin(); it != m_list.end();)
	{
		if(it->message->IsType(type) ||  type == CDVDMsg::NONE)
			it = m_list.erase(it);
		else
			it++;
	}

	if(type == CDVDMsg::DEMUXER_PACKET ||  type == CDVDMsg::NONE)
	{
		m_iDataSize = 0;
		m_TimeBack  = DVD_NOPTS_VALUE;
		m_TimeFront = DVD_NOPTS_VALUE;
		m_bEmptied = true;
	}
}

void CDVDMessageQueue::Abort()
{
	CSingleLock lock(m_section);

	m_bAbortRequest = true;

	SetEvent(m_hEvent); // Inform waiter for abort action
}

void CDVDMessageQueue::End()
{
	CSingleLock lock(m_section);

	Flush();
  
	m_bInitialized  = false;
	m_iDataSize     = 0;
	m_bAbortRequest = false;
}

MsgQueueReturnCode CDVDMessageQueue::Get(CDVDMsg** pMsg, unsigned int iTimeoutInMilliSeconds, int priority)
{
	CSingleLock lock(m_section);

	*pMsg = NULL;
  
	int ret = 0;

	if(!m_bInitialized)
	{
		CLog::Log(LOGFATAL, "CDVDMessageQueue(%s)::Get MSGQ_NOT_INITIALIZED", m_owner.c_str());
		return MSGQ_NOT_INITIALIZED;
	}

	if(m_list.empty() && m_bEmptied == false && priority == 0 && m_owner != "teletext")
	{
		CLog::Log(LOGWARNING, "CDVDMessageQueue(%s)::Get - asked for new data packet, with nothing available", m_owner.c_str());
		m_bEmptied = true;
	}

	while(!m_bAbortRequest)
	{
		if(!m_list.empty() && m_list.back().priority >= priority && !m_bCaching)
		{
			DVDMessageListItem& item(m_list.back());
      
			if(item.message->IsType(CDVDMsg::DEMUXER_PACKET))
			{
				DemuxPacket* packet = ((CDVDMsgDemuxerPacket*)item.message)->GetPacket();
				if(packet)
				{
					m_iDataSize -= packet->iSize;

					if(packet->dts != DVD_NOPTS_VALUE)
						m_TimeBack = packet->dts;
					else if(packet->pts != DVD_NOPTS_VALUE)
						m_TimeBack = packet->pts;
				}

				if(m_bEmptied && m_iDataSize > 0)
					m_bEmptied = false;
			}

			*pMsg = item.message->Acquire();
			m_list.pop_back();
      
			ret = MSGQ_OK;
			break;
		}
		else if(!iTimeoutInMilliSeconds)
		{
			ret = MSGQ_TIMEOUT;
			break;
		}
		else
		{
			ResetEvent(m_hEvent);
			lock.Leave();
      
			// Wait for a new message
			if (WaitForSingleObject(m_hEvent, iTimeoutInMilliSeconds) == WAIT_TIMEOUT)
				return MSGQ_TIMEOUT;

			lock.Enter();
		}
	}
  
	if(m_bAbortRequest) return MSGQ_ABORT;
  
	return(MsgQueueReturnCode)ret;
}

MsgQueueReturnCode CDVDMessageQueue::Put(CDVDMsg* pMsg, int priority)	
{
	CSingleLock lock(m_section);

	if(!m_bInitialized)
	{
		CLog::Log(LOGWARNING, "CDVDMessageQueue(%s)::Put MSGQ_NOT_INITIALIZED", m_owner.c_str());
		pMsg->Release();
		return MSGQ_NOT_INITIALIZED;
	}

	if(!pMsg)
	{
		CLog::Log(LOGFATAL, "CDVDMessageQueue(%s)::Put MSGQ_INVALID_MSG", m_owner.c_str());
		return MSGQ_INVALID_MSG;
	}

	SList::iterator it = m_list.begin();
	while(it != m_list.end())
	{
		if(priority <= it->priority)
			break;
		it++;
	}

	m_list.insert(it, DVDMessageListItem(pMsg, priority));

	if(pMsg->IsType(CDVDMsg::DEMUXER_PACKET))
	{
		DemuxPacket* packet = ((CDVDMsgDemuxerPacket*)pMsg)->GetPacket();
		if(packet)
		{
			m_iDataSize += packet->iSize;

			if(packet->dts != DVD_NOPTS_VALUE)
				m_TimeFront = packet->dts;
			else if(packet->pts != DVD_NOPTS_VALUE)
				m_TimeFront = packet->pts;
			if(m_TimeBack == DVD_NOPTS_VALUE)
				m_TimeBack = m_TimeFront;
		}
	}

	pMsg->Release();

	SetEvent(m_hEvent); // Inform waiter for new packet

	return MSGQ_OK;
}

unsigned CDVDMessageQueue::GetPacketCount(CDVDMsg::Message type)
{    
	CSingleLock lock(m_section);

	if(!m_bInitialized)
		return 0;

	unsigned count = 0;
	for(SList::iterator it = m_list.begin(); it != m_list.end();it++)
	{
		if(it->message->IsType(type))
			count++;
	}
  
	return count;
}

void CDVDMessageQueue::WaitUntilEmpty()
{
	CLog::Log(LOGNOTICE, "CDVDMessageQueue(%s)::WaitUntilEmpty", m_owner.c_str());
	CDVDMsgGeneralSynchronize* msg = new CDVDMsgGeneralSynchronize(40000, 0);
    Put(msg->Acquire());
	msg->Wait(&m_bAbortRequest, 0);
	msg->Release();
}

int CDVDMessageQueue::GetLevel() const
{
	if(m_iDataSize > m_iMaxDataSize)
		return 100;

	if(m_iDataSize == 0)
		return 0;

	if(m_TimeBack  == DVD_NOPTS_VALUE
	|| m_TimeFront == DVD_NOPTS_VALUE
	|| m_TimeFront <= m_TimeBack)
		return min(100, 100 * m_iDataSize / m_iMaxDataSize);

	double dTmp = 100.0 * m_TimeSize * (m_TimeFront - m_TimeBack) / DVD_TIME_BASE;
	int iTmpRound = (int)(dTmp + 0.5);
	return min(100, iTmpRound /*MathUtils::round_int(100.0 * m_TimeSize * (m_TimeFront - m_TimeBack) / DVD_TIME_BASE )*/);
}
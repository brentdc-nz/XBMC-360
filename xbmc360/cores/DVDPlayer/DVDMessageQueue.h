#ifndef H_CDVDMESSAGEQUEUE
#define H_CDVDMESSAGEQUEUE

#include <string>
#include "DVDMessage.h"
#include "utils\SingleLock.h"
#include <string>
#include <list>
#include "utils\CriticalSection.h"

struct DVDMessageListItem
{
	DVDMessageListItem(CDVDMsg* msg, int prio)
	{
		message  = msg->Acquire();
		priority = prio;
	}

	DVDMessageListItem()
	{
		message  = NULL;
		priority = 0;
	}

	DVDMessageListItem(const DVDMessageListItem& item)
	{
		if(item.message)
			message = item.message->Acquire();
		else
			message = NULL;

		priority = item.priority;
	}

	~DVDMessageListItem()
	{
		if(message)
			message->Release();
	}

	DVDMessageListItem& operator=(const DVDMessageListItem& item)
	{
		if(message)
			message->Release();

		if(item.message)
			message = item.message->Acquire();
		else
			message = NULL;

		priority = item.priority;
		return *this;
	}

	CDVDMsg* message;
	int      priority;
};

enum MsgQueueReturnCode
{
	MSGQ_OK               = 1,
	MSGQ_TIMEOUT          = 0,
	MSGQ_ABORT            = -1, // Negative for legacy, not an error actually
	MSGQ_NOT_INITIALIZED  = -2,
	MSGQ_INVALID_MSG      = -3,
	MSGQ_OUT_OF_MEMORY    = -4
};

#define MSGQ_IS_ERROR(c) (c < 0)

class CDVDMessageQueue
{
public:
	CDVDMessageQueue(const std::string &owner);
	virtual ~CDVDMessageQueue();

	void  Init();
	void  Flush(CDVDMsg::Message message = CDVDMsg::DEMUXER_PACKET);
	void  Abort();
	void  End();

	/**
	* msg,       message type from DVDMessage.h
	* timeout,   timeout in msec
	*/
	MsgQueueReturnCode Get(CDVDMsg** pMsg, unsigned int iTimeoutInMilliSeconds, int priority = 0);
	MsgQueueReturnCode Put(CDVDMsg* pMsg, int priority = 0);
	unsigned GetPacketCount(CDVDMsg::Message type);
	void WaitUntilEmpty();
	void SetMaxDataSize(int iMaxDataSize) { m_iMaxDataSize = iMaxDataSize; }
	void SetMaxTimeSize(double sec)       { m_TimeSize  = 1.0 / /*std::*/max(1.0, sec); }
	int GetMaxDataSize() const            { return m_iMaxDataSize; }
	bool IsInited() const                 { return m_bInitialized; }
	int GetDataSize() const               { return m_iDataSize; }
	bool ReceivedAbortRequest()           { return m_bAbortRequest; }
	// Non messagequeue related functions
	bool IsFull() const                   { return GetLevel() == 100; }
	int  GetLevel() const;

private:
	HANDLE m_hEvent;
	mutable CCriticalSection m_section;
  
	bool m_bAbortRequest;
	bool m_bInitialized;
	bool m_bCaching;

	int m_iDataSize;
	double m_TimeFront;
	double m_TimeBack;
	double m_TimeSize;

	int m_iMaxDataSize;
	bool m_bEmptied;
	std::string m_owner;

	typedef std::list<DVDMessageListItem> SList;
	SList m_list;
};

#endif //H_CDVDMESSAGEQUEUE
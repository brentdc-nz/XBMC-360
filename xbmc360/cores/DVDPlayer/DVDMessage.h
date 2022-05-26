#ifndef H_CDVDMSG
#define H_CDVDMSG

#include "DVDDemuxers/DVDDemux.h"
#include <assert.h>

class CDVDMsg
{
public:
	enum Message
	{
		NONE = 1000,
 
		// Messages used in the whole system
		GENERAL_RESYNC,
		GENERAL_FLUSH,                  // Flush all buffers
		GENERAL_RESET,                  // Reset codecs for new data
		GENERAL_STREAMCHANGE,           // 
		GENERAL_SYNCHRONIZE,            // 
		GENERAL_DELAY,                  //
		GENERAL_GUI_ACTION,             // Gui action of some sort
		GENERAL_EOF,                    // eof of stream

		// Player core related messages (cdvdplayer.cpp)
		PLAYER_SET_AUDIOSTREAM,         
		PLAYER_SET_SUBTITLESTREAM,      
		PLAYER_SET_SUBTITLESTREAM_VISIBLE,
		PLAYER_SET_STATE,               // Restore the dvdplayer to a certain state
		PLAYER_SET_RECORD,              // Set record state
		PLAYER_SEEK,                    // 
		PLAYER_SEEK_CHAPTER,            //
		PLAYER_SETSPEED,                // Set the playback speed

		PLAYER_CHANNEL_NEXT,            // Switches to next playback channel
		PLAYER_CHANNEL_PREV,            // Switches to previous playback channel
		PLAYER_CHANNEL_SELECT,          // Switches to given playback channel
		PLAYER_STARTED,                 // Sent whenever a sub player has finished it's first frame after open

		// Demuxer related messages
		DEMUXER_PACKET,                 // Data packet
		DEMUXER_RESET,                  // Reset the demuxer
    
		// Video related messages
		VIDEO_NOSKIP,                   // Next pictures is not to be skipped by the video renderer
		VIDEO_SET_ASPECT,               // Set aspectratio of video

		// Subtitle related messages
		SUBTITLE_CLUTCHANGE
	};

	CDVDMsg(Message msg)
	{
		m_references = 1;
		m_message = msg;
#ifdef DVDDEBUG_MESSAGE_TRACKER
		g_dvdMessageTracker.Register(this);
#endif
	}

	CDVDMsg(Message msg, long references)
	{
		m_references = references;
		m_message = msg;
#ifdef DVDDEBUG_MESSAGE_TRACKER
		g_dvdMessageTracker.Register(this);
#endif
	}
  
	virtual ~CDVDMsg()
	{
		assert(m_references == 0); 
#ifdef DVDDEBUG_MESSAGE_TRACKER
		g_dvdMessageTracker.UnRegister(this);
#endif
	}

	/**
	* Increase the reference counter by one.   
	*/
	long Release()
	{
		long count = InterlockedDecrement(&m_references);
		if(count == 0) delete this;
		return count;
	}

	/**
	* decrease the reference counter by one.   
	*/
	CDVDMsg* Acquire()
	{
		InterlockedIncrement(&m_references);
		return this;
	}

	/**
	* checks for message type
	*/
	bool IsType(Message msg)
	{
		return (m_message == msg);
	}

	long GetNrOfReferences()
	{
		return m_references;
	}

private:
	long m_references;
	Message m_message;
};

////////////////////////////////////////////////////////////////////////////////
//////
////// PLAYER_ Messages
//////
////////////////////////////////////////////////////////////////////////////////

class CDVDMsgPlayerSeek : public CDVDMsg
{
public:
	CDVDMsgPlayerSeek(int time, bool backward, bool flush = true, bool accurate = true, bool restore = true)
		: CDVDMsg(PLAYER_SEEK)
		, m_time(time)
		, m_backward(backward)
		, m_flush(flush)
		, m_accurate(accurate)
		, m_restore(restore)
	{}

	int  GetTime()              { return m_time; }
	bool GetBackward()          { return m_backward; }
	bool GetFlush()             { return m_flush; }
	bool GetAccurate()          { return m_accurate; }
	bool GetRestore()           { return m_restore; }

private:
	int  m_time;
	bool m_backward;
	bool m_flush;
	bool m_accurate;
	bool m_restore; // Whether to restore any EDL cut time
};


////////////////////////////////////////////////////////////////////////////////
//////
////// DEMUXER_ Messages
//////
////////////////////////////////////////////////////////////////////////////////

class CDVDMsgDemuxerPacket : public CDVDMsg
{
public:
	CDVDMsgDemuxerPacket(DemuxPacket* packet, bool drop = false);
	virtual ~CDVDMsgDemuxerPacket();

	DemuxPacket* GetPacket()      { return m_packet; }
	unsigned int GetPacketSize()  { if(m_packet) return m_packet->iSize; else return 0; }
	bool         GetPacketDrop()  { return m_drop; }

private:
	DemuxPacket* m_packet;
	bool         m_drop;
};

////////////////////////////////////////////////////////////////////////////////
//////
////// GENERAL_ Messages
//////
////////////////////////////////////////////////////////////////////////////////

class CDVDMsgGeneralResync : public CDVDMsg
{
public:
	CDVDMsgGeneralResync(double timestamp, bool clock) : CDVDMsg(GENERAL_RESYNC)  { m_timestamp = timestamp; m_clock = clock; }
	double m_timestamp;
	bool m_clock;
};

#define SYNCSOURCE_AUDIO  0x00000001
#define SYNCSOURCE_VIDEO  0x00000002
#define SYNCSOURCE_SUB    0x00000004
#define SYNCSOURCE_ALL    (SYNCSOURCE_AUDIO | SYNCSOURCE_VIDEO | SYNCSOURCE_SUB)

class CDVDMsgGeneralSynchronize : public CDVDMsg
{
public:
	CDVDMsgGeneralSynchronize(DWORD timeout, DWORD sources);

	// Waits until all threads waiting, released the object
	// if abort is set somehow
	void Wait(volatile bool *abort, DWORD source);

private:
	DWORD m_sources;
	long m_objects;
	unsigned int m_timeout;
};

template <typename T>

class CDVDMsgType : public CDVDMsg
{
public:
	CDVDMsgType(Message type, T value) 
		: CDVDMsg(type) 
		, m_value(value)
	{}
	operator T() { return m_value; }
	T m_value;
};

typedef CDVDMsgType<bool>   CDVDMsgBool;
typedef CDVDMsgType<int>    CDVDMsgInt;
typedef CDVDMsgType<double> CDVDMsgDouble;

#endif //H_CDVDMSG
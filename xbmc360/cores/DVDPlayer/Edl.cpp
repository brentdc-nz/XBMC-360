#include "Edl.h"
#include "utils\Log.h"
#include "utils\StringUtils.h"
#include "filesystem\File.h"

#define MPLAYER_EDL_FILENAME "special://temp/xbmc.edl"

using namespace XFILE;

CEdl::CEdl()
{
	Clear();
}

CEdl::~CEdl()
{
	Clear();
}

void CEdl::Clear()
{
	if(CFile::Exists(MPLAYER_EDL_FILENAME))
	{}//	CFile::Delete(MPLAYER_EDL_FILENAME); //TODO

	m_vecCuts.clear();
	m_vecSceneMarkers.clear();
	m_iTotalCutTime = 0;
}

bool CEdl::ReadEditDecisionLists(const CStdString& strMovie, const float fFrameRate, const int iHeight) //TODO
{
	/*
	* The frame rate hints returned from ffmpeg for the video stream do not appear to take into
	* account whether the content is interlaced. This affects the calculation to time offsets based
	* on frames per second as most commercial detection programs use full frames, which need two
	* interlaced fields to calculate a single frame so the actual frame rate is half.
	*
	* Adjust the frame rate using the detected frame rate or height to determine typical interlaced
	* content (obtained from http://en.wikipedia.org/wiki/Frame_rate)
	*
	* Note that this is a HACK and we should be able to get the frame rate from the source sending
	* back frame markers. However, this doesn't seem possible for MythTV.
	*/
	float fFramesPerSecond;
	if(int(fFrameRate * 100) == 5994) // 59.940 fps = NTSC or 60i content
	{
		fFramesPerSecond = fFrameRate / 2; // ~29.97f - Division used to retain accuracy of original.
		CLog::Log(LOGDEBUG, "%s - Assuming NTSC or 60i interlaced content. Adjusted frames per second from %.3f (~59.940 fps) to %.3f",
				__FUNCTION__, fFrameRate, fFramesPerSecond);
	}
	else if(int(fFrameRate * 100) == 4795) // 47.952 fps = 24p -> NTSC conversion
	{
		fFramesPerSecond = fFrameRate / 2; // ~23.976f - Division used to retain accuracy of original.
		CLog::Log(LOGDEBUG, "%s - Assuming 24p -> NTSC conversion interlaced content. Adjusted frames per second from %.3f (~47.952 fps) to %.3f",
				__FUNCTION__, fFrameRate, fFramesPerSecond);
	}
	else if (iHeight == 576 && fFrameRate > 30.0) // PAL @ 50.0fps rather than PAL @ 25.0 fps. Can't use direct fps check of 50.0 as this is valid for 720p
	{
		fFramesPerSecond = fFrameRate / 2; // ~25.0f - Division used to retain accuracy of original.
		CLog::Log(LOGDEBUG, "%s - Assuming PAL interlaced content. Adjusted frames per second from %.3f (~50.00 fps) to %.3f",
				__FUNCTION__, fFrameRate, fFramesPerSecond);
	}
	else if (iHeight == 1080 && fFrameRate > 30.0) // Don't know of any 1080p content being broadcast at higher than 30.0 fps so assume 1080i
	{
		fFramesPerSecond = fFrameRate / 2;
		CLog::Log(LOGDEBUG, "%s - Assuming 1080i interlaced content. Adjusted frames per second from %.3f to %.3f",
			__FUNCTION__, fFrameRate, fFramesPerSecond);
	}
	else // Assume everything else is not interlaced, e.g. 720p.
		fFramesPerSecond = fFrameRate;

	bool bFound = false;

#if 0 //TODO

	/*
	* Only check for edit decision lists if the movie is on the local hard drive, or accessed over a
	* network share.
	*/
	if (CUtil::IsHD(strMovie)
	||  CUtil::IsSmb(strMovie))
	{
		CLog::Log(LOGDEBUG, "%s - Checking for edit decision lists (EDL) on local drive or remote share for: %s",
				__FUNCTION__, strMovie.c_str());
		/*
		* Read any available file format until a valid EDL related file is found.
		*/
		if(!bFound)
			bFound = ReadVideoReDo(strMovie);

		if(!bFound)
			bFound = ReadEdl(strMovie, fFramesPerSecond);

		if(!bFound)
			bFound = ReadComskip(strMovie, fFramesPerSecond);

		if(!bFound)
			bFound = ReadBeyondTV(strMovie);
	}
	/*
	* Or if the movie points to MythTV and isn't live TV.
	*/
	else if (CUtil::IsMythTV(strMovie)
	&&      !CUtil::IsLiveTV(strMovie))
	{
		CLog::Log(LOGDEBUG, "%s - Checking for commercial breaks within MythTV for: %s", __FUNCTION__,
				strMovie.c_str());

		bFound = ReadMythCommBreaks(strMovie, fFramesPerSecond);
	}

	if (bFound)
	{
		MergeShortCommBreaks();
		WriteMPlayerEdl();
	}
#endif
	return bFound;
}

bool CEdl::InCut(const int64_t iSeek, Cut *pCut)
{
	for (int i = 0; i < (int)m_vecCuts.size(); i++)
	{
		if(iSeek < m_vecCuts[i].start) // Early exit if not even up to the cut start time.
			return false;

		if(iSeek >= m_vecCuts[i].start && iSeek <= m_vecCuts[i].end) // Inside cut.
		{
			if(pCut)
				*pCut = m_vecCuts[i];

			return true;
		}
	}
	return false;
}

bool CEdl::HasCut()
{
	return !m_vecCuts.empty();
}

bool CEdl::HasSceneMarker()
{
	return !m_vecSceneMarkers.empty();
}

int64_t CEdl::RemoveCutTime(int64_t iSeek)
{
	if(!HasCut())
		return iSeek;

	/*
	* TODO: Consider an optimisation of using the (now unused) total cut time if the seek time
	* requested is later than the end of the last recorded cut. For example, when calculating the
	* total duration for display.
	*/
	int64_t iCutTime = 0;
	for(int i = 0; i < (int)m_vecCuts.size(); i++)
	{
		if(m_vecCuts[i].action == CUT)
		{
			if(iSeek >= m_vecCuts[i].start && iSeek <= m_vecCuts[i].end) // Inside cut
				iCutTime += iSeek - m_vecCuts[i].start - 1; // Decrease cut length by 1ms to jump over end boundary.
			else if (iSeek >= m_vecCuts[i].start) // Cut has already been passed over.
				iCutTime += m_vecCuts[i].end - m_vecCuts[i].start;
		}
	}
	return iSeek - iCutTime;
}

int64_t CEdl::RestoreCutTime(int64_t iClock)
{
	if(!HasCut())
		return iClock;

	int64_t iSeek = iClock;
	for (int i = 0; i < (int)m_vecCuts.size(); i++)
	{
		if(m_vecCuts[i].action == CUT && iSeek >= m_vecCuts[i].start)
			iSeek += m_vecCuts[i].end - m_vecCuts[i].start;
	}

	return iSeek;
}

CStdString CEdl::GetInfo()
{
	CStdString strInfo = "";
	if(HasCut())
	{
		int cutCount = 0, muteCount = 0, commBreakCount = 0;
		for (int i = 0; i < (int)m_vecCuts.size(); i++)
		{
			switch (m_vecCuts[i].action)
			{
				case CUT:
					cutCount++;
					break;
				case MUTE:
					muteCount++;
					break;
				case COMM_BREAK:
					commBreakCount++;
					break;
			}
		}
		
		if(cutCount > 0)
			strInfo.AppendFormat("c%i", cutCount);
		if(muteCount > 0)
			strInfo.AppendFormat("m%i", muteCount);
		if(commBreakCount > 0)
			strInfo.AppendFormat("b%i", commBreakCount);
	}

	if(HasSceneMarker())
		strInfo.AppendFormat("s%i", m_vecSceneMarkers.size());

	return strInfo.IsEmpty() ? "-" : strInfo;
}

CStdString CEdl::MillisecondsToTimeString(const int64_t iMilliseconds)
{
	CStdString strTimeString = CStringUtils::SecondsToTimeString((long)(iMilliseconds / 1000), TIME_FORMAT_HH_MM_SS); // milliseconds to seconds
	strTimeString.AppendFormat(".%03i", iMilliseconds % 1000);
	return strTimeString;
}
/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "DVDFileInfo.h"
#include "DVDDemuxers\DVDDemux.h"
#include "utils\StreamDetails.h"

/* returns true if details have been added */
bool CDVDFileInfo::DemuxerToStreamDetails(CDVDDemux *pDemux, CStreamDetails &details, const CStdString &path)
{
  bool retVal = false;
  details.Reset();

  for (int iStream=0; iStream<pDemux->GetNrOfStreams(); iStream++)
  {
    CDemuxStream *stream = pDemux->GetStream(iStream);
    if (stream->type == STREAM_VIDEO)
    {
      CStreamDetailVideo *p = new CStreamDetailVideo();
      p->m_iWidth = ((CDemuxStreamVideo *)stream)->iWidth;
      p->m_iHeight = ((CDemuxStreamVideo *)stream)->iHeight;
      p->m_fAspect = ((CDemuxStreamVideo *)stream)->fAspect;
      if (p->m_fAspect == 0.0f)
        p->m_fAspect = (float)p->m_iWidth / p->m_iHeight;
      pDemux->GetStreamCodecName(iStream, p->m_strCodec);
      p->m_iDuration = pDemux->GetStreamLength();

      // finally, calculate seconds
      if (p->m_iDuration > 0)
        p->m_iDuration = p->m_iDuration / 1000;

      details.AddStream(p);
      retVal = true;
    }

    else if (stream->type == STREAM_AUDIO)
    {
      CStreamDetailAudio *p = new CStreamDetailAudio();
      p->m_iChannels = ((CDemuxStreamAudio *)stream)->iChannels;
      p->m_strLanguage = stream->language;
      pDemux->GetStreamCodecName(iStream, p->m_strCodec);
      details.AddStream(p);
      retVal = true;
    }

    else if (stream->type == STREAM_SUBTITLE)
    {
      if (stream->language)
      {
        CStreamDetailSubtitle *p = new CStreamDetailSubtitle();
        p->m_strLanguage = stream->language;
        details.AddStream(p);
        retVal = true;
      }
    }
  }  /* for iStream */

  details.DetermineBestStreams();
  return retVal;
}

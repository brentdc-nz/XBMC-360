#include "utils\Log.h"
#include "ZipFile.h"
#include "URL.h"
#include "Application.h"

#include <sys/stat.h>

using namespace XFILE;
using namespace std;

CZipFile::CZipFile() : m_dlgProgress(NULL), m_bUseProgressBar(false)
{
	m_szStringBuffer = NULL;
	m_szStartOfStringBuffer = NULL;
	m_iDataInStringBuffer = 0;
	m_iRead = -1;
}

CZipFile::~CZipFile()
{
	if (m_szStringBuffer)
		delete[] m_szStringBuffer;

	Close();
}

bool CZipFile::Open(const CURL&url)
{
	CStdString strOpts = url.GetOptions();
	CURL url2(url);
	url2.SetOptions("");
	CStdString strPath = url2.Get();

	if (!g_ZipManager.GetZipEntry(strPath,mZipItem))
		return false;

	if ((mZipItem.flags & 64) == 64)
	{
		CLog::Log(LOGERROR,"FileZip: encrypted file, not supported!");
		return false;
	}

	if ((mZipItem.method != 8) && (mZipItem.method != 0))
	{
		CLog::Log(LOGERROR,"FileZip: unsupported compression method!");
		return false;
	}

	if (!mFile.Open(url.GetHostName())) // this is the zip-file, always open binary
	{
		CLog::Log(LOGERROR,"FileZip: unable to open zip file %s!",url.GetHostName().c_str());
		return false;
	}
	mFile.Seek(mZipItem.offset,SEEK_SET);

	return InitDecompress();
}

bool CZipFile::InitDecompress()
{
	m_iRead = 1;
	m_iFilePos = 0;
	m_iZipFilePos = 0;
	m_iAvailBuffer = 0;
	m_bFlush = false;
	m_ZStream.zalloc = Z_NULL;
	m_ZStream.zfree = Z_NULL;
	m_ZStream.opaque = Z_NULL;

	if( mZipItem.method != 0 )
	{
		if (inflateInit2(&m_ZStream,-MAX_WBITS) != Z_OK)
		{
			CLog::Log(LOGERROR,"FileZip: error initializing zlib!");
			return false;
		}
	}

	m_ZStream.next_in = (Bytef*)m_szBuffer;
	m_ZStream.avail_in = 0;
	m_ZStream.total_out = 0;

	return true;
}

int64_t CZipFile::GetLength()
{
	return mZipItem.usize;
}

int64_t CZipFile::GetPosition()
{
	return m_iFilePos;
}

int64_t CZipFile::Seek(int64_t iFilePosition, int iWhence)
{
	if (mZipItem.method == 0) // this is easy
	{
		int64_t iResult;
		switch (iWhence)
		{
		case SEEK_SET:
			if (iFilePosition > mZipItem.usize)
				return -1;
			m_iFilePos = iFilePosition;
			m_iZipFilePos = m_iFilePos;
			iResult = mFile.Seek(iFilePosition+mZipItem.offset,SEEK_SET)-mZipItem.offset;
			return iResult;
			break;

		case SEEK_CUR:
			if (m_iFilePos+iFilePosition > mZipItem.usize)
				return -1;
			m_iFilePos += iFilePosition;
			m_iZipFilePos = m_iFilePos;
			iResult = mFile.Seek(iFilePosition,SEEK_CUR)-mZipItem.offset;
			return iResult;
			break;

		case SEEK_END:
			if (iFilePosition > mZipItem.usize)
				return -1;
			m_iFilePos = mZipItem.usize+iFilePosition;
			m_iZipFilePos = m_iFilePos;
			iResult = mFile.Seek(mZipItem.offset+mZipItem.usize+iFilePosition,SEEK_SET)-mZipItem.offset;
			return iResult;
			break;
		default:
			return -1;

		}
	}

	// here goes the stupid part..
	if (mZipItem.method == 8)
	{
		// Scratch must be a member, not a local: a forward SEEK_SET recurses
		// into SEEK_CUR, and two 128k stack frames blow the 256k default
		// thread stack of release builds
		char* temp = m_szSeekBuffer;
		int64_t iStartPos = m_iFilePos;

		switch (iWhence)
		{
		case SEEK_SET:
			if (iFilePosition == m_iFilePos)
				return m_iFilePos; // mp3reader does this lots-of-times
			if (iFilePosition > mZipItem.usize || iFilePosition < 0)
				return -1;
			// read until position in 128k blocks.. only way to do it due to format.
			// can't start in the middle of data since then we'd have no clue where
			// we are in uncompressed data..
			if (iFilePosition < m_iFilePos)
			{
				m_iFilePos = 0;
				m_iZipFilePos = 0;
				inflateEnd(&m_ZStream);
				inflateInit2(&m_ZStream,-MAX_WBITS); // simply restart zlib
				mFile.Seek(mZipItem.offset,SEEK_SET);
				m_ZStream.next_in = (Bytef*)m_szBuffer;
				m_ZStream.avail_in = 0;
				m_ZStream.total_out = 0;
				if (iFilePosition > 1024*1024) // 1 MB seek
				{
					StartProgressBar();
				}
				while (m_iFilePos < iFilePosition)
				{
					unsigned int iToRead = (iFilePosition-m_iFilePos)>131072?131072:(int)(iFilePosition-m_iFilePos);
					if (Read(temp,iToRead) != iToRead)
						return -1;
					if (m_bUseProgressBar)
					{
						m_dlgProgress->SetPercentage(static_cast<int>(static_cast<float>(m_iFilePos)/static_cast<float>(iFilePosition)*100));
						m_dlgProgress->Progress();
					}
				}
				if( m_bUseProgressBar)
				{
					StopProgressBar();
					m_bUseProgressBar = false;
				}
				return m_iFilePos;
			}
			else // seek forward
				return Seek(iFilePosition-m_iFilePos,SEEK_CUR);
			break;

		case SEEK_CUR:
			if (iFilePosition < 0)
				return Seek(m_iFilePos+iFilePosition,SEEK_SET); // can't rewind stream
			// read until requested position, drop data
			if (m_iFilePos+iFilePosition > mZipItem.usize)
				return -1;
			iFilePosition += m_iFilePos;
			if (iFilePosition-m_iFilePos > 1024*1024) // 1 MB seek
			{
				StartProgressBar();
			}
			while (m_iFilePos < iFilePosition)
			{
				unsigned int iToRead = (iFilePosition-m_iFilePos)>131072?131072:(int)(iFilePosition-m_iFilePos);
				if (Read(temp,iToRead) != iToRead)
					return -1;
				if (m_bUseProgressBar)
				{
					m_dlgProgress->SetPercentage(static_cast<int>(static_cast<float>(m_iFilePos)/static_cast<float>(iFilePosition)*100));
					m_dlgProgress->Progress();
				}
			}
			if( m_bUseProgressBar)
				{
					StopProgressBar();
					m_bUseProgressBar = false;
				}
			return m_iFilePos;
			break;

		case SEEK_END:
			// now this is a nasty bastard, possibly takes lotsoftime
			// uncompress, minding m_ZStream.total_out

			if ((GetLength()+iFilePosition)-m_iFilePos > 1024*1024) // 1 MB seek
			{
				StartProgressBar();
			}

			while( m_ZStream.total_out < mZipItem.usize+iFilePosition)
			{
				unsigned int iToRead = (mZipItem.usize+iFilePosition-m_ZStream.total_out > 131072)?131072:(int)(mZipItem.usize+iFilePosition-m_ZStream.total_out);
				if (Read(temp,iToRead) != iToRead)
					return -1;
				if (m_bUseProgressBar)
				{
					m_dlgProgress->SetPercentage(static_cast<int>(static_cast<float>(m_iFilePos-iStartPos)/static_cast<float>(mZipItem.usize+iFilePosition)*100));
					m_dlgProgress->Progress();
				}
			}
			if( m_bUseProgressBar)
			{
					StopProgressBar();
					m_bUseProgressBar = false;
			}
			return m_iFilePos;
			break;
		default:
			return -1;
		}
	}
	return -1;
}

bool CZipFile::Exists(const CURL& url)
{
	SZipEntry item;

	if (g_ZipManager.GetZipEntry(url.Get(),item))
		return true;

	return false;
}

int CZipFile::Stat(const CURL& url, struct __stat64* buffer)
{
	if (!g_ZipManager.GetZipEntry(url.Get(),mZipItem))
		return -1;

	memset(buffer, 0, sizeof(struct __stat64));
	buffer->st_gid = 0;
	buffer->st_atime = buffer->st_ctime = mZipItem.mod_time;
	buffer->st_size = mZipItem.usize;

	return -1;
}

unsigned int CZipFile::Read(void* lpBuf, int64_t uiBufSize)
{
	// flush what might be left in the string buffer
	if (m_iDataInStringBuffer > 0)
	{
		size_t iMax = static_cast<size_t>((uiBufSize>m_iDataInStringBuffer?m_iDataInStringBuffer:uiBufSize));
		memcpy(lpBuf,m_szStartOfStringBuffer,iMax);
		uiBufSize -= iMax;
		m_iDataInStringBuffer -= iMax;
	}

	if (mZipItem.method == 8) // deflated
	{
		uLong iDecompressed = 0;
		uLong prevOut = m_ZStream.total_out;
		while (((int)iDecompressed < uiBufSize) && ((m_iZipFilePos < mZipItem.csize) || (m_bFlush)))
		{
			m_ZStream.next_out = (Bytef*)(lpBuf)+iDecompressed;
			m_ZStream.avail_out = static_cast<uInt>(uiBufSize-iDecompressed);
			if (m_bFlush) // need to flush buffer !
			{
				int iMessage = inflate(&m_ZStream,Z_SYNC_FLUSH);
				m_bFlush = ((iMessage == Z_OK) && (m_ZStream.avail_out == 0))?true:false;
				if (!m_ZStream.avail_out) // flush filled buffer, get out of here
				{
					iDecompressed = m_ZStream.total_out-prevOut;
					break;
				}
			}

			if (!m_ZStream.avail_in)
			{
				if (!FillBuffer()) // eof!
				{
					iDecompressed = m_ZStream.total_out-prevOut;
					break;
				}
			}

			int iMessage = inflate(&m_ZStream,Z_SYNC_FLUSH);
			if (iMessage < 0)
			{
				Close();
				return 0; // READ ERROR
			}

			m_bFlush = ((iMessage == Z_OK) && (m_ZStream.avail_out == 0))?true:false; // more info in input buffer

			iDecompressed = m_ZStream.total_out-prevOut;
		}
		m_iFilePos += iDecompressed;

		return static_cast<unsigned int>(iDecompressed);
	}
	else if (mZipItem.method == 0) // uncompressed. just read from file, but mind our boundaries.
	{
		if (uiBufSize+m_iFilePos > mZipItem.csize)
			uiBufSize = mZipItem.csize-m_iFilePos;
		if (uiBufSize < 0)
		{
			return 0; // we are past eof, this shouldn't happen but test anyway
		}
		unsigned int iResult = mFile.Read(lpBuf,uiBufSize);
		m_iZipFilePos += iResult;
		m_iFilePos += iResult;
		return iResult;
	}
	else
		return false; // shouldn't happen. compression method checked in open
}

void CZipFile::Close()
{
	if (mZipItem.method == 8 && m_iRead != -1)
		inflateEnd(&m_ZStream);

	mFile.Close();
}

bool CZipFile::FillBuffer()
{
	unsigned int sToRead = 65535;

	if (m_iZipFilePos+65535 > mZipItem.csize)
		sToRead = static_cast<int>(mZipItem.csize-m_iZipFilePos);

	if (sToRead <= 0)
		return false; // eof!

	if (mFile.Read(m_szBuffer,sToRead) != sToRead)
		return false;

	m_ZStream.avail_in = sToRead;
	m_ZStream.next_in = (Bytef*)m_szBuffer;
	m_iZipFilePos += sToRead;

	return true;
}

void CZipFile::DestroyBuffer(void* lpBuffer, int iBufSize)
{
	if (!m_bFlush)
		return;

	int iMessage = Z_STREAM_END; // whatever != Z_OK

	while ((iMessage == Z_OK) && (m_ZStream.avail_out == 0))
	{
		m_ZStream.next_out = (Bytef*)lpBuffer;
		m_ZStream.avail_out = iBufSize;
		iMessage = inflate(&m_ZStream,Z_SYNC_FLUSH);
	}

	m_bFlush = false;
}

void CZipFile::StartProgressBar()
{
	// Seeks run on the player's reader thread. Driving a modal dialog from
	// there renders on a thread that does not own the D3D device, which the
	// XDK debug layer breaks on (ERR(D3D) thread ownership). Only drive the
	// dialog when called from the application thread; otherwise the seek
	// simply proceeds without a progress dialog.
	m_bUseProgressBar = false;

	if (!g_application.IsCurrentThread())
		return;

	if (!m_dlgProgress)
		m_dlgProgress = (CGUIDialogProgress*)g_windowManager.GetWindow(WINDOW_DIALOG_PROGRESS);

	m_dlgProgress->StartModal();
	m_dlgProgress->SetPercentage(0);
	m_dlgProgress->SetHeading(773);
	m_dlgProgress->SetLine(0,"");
	m_dlgProgress->SetLine(1,"");
	m_dlgProgress->SetLine(2,"");
	m_dlgProgress->ShowProgressBar(true);
	m_bUseProgressBar = true;
}

void CZipFile::StopProgressBar()
{
	if (m_dlgProgress)
		m_dlgProgress->Close();
}

int CZipFile::UnpackFromMemory(std::string& strDest, const std::string& strInput, bool isGZ)
{
	unsigned int iPos=0;
	int iResult=0;

	while( iPos+LHDR_SIZE < strInput.size() || isGZ)
	{
		if (!isGZ)
		{
			CZipManager::readHeader(strInput.data()+iPos,mZipItem);
			if( mZipItem.header != ZIP_LOCAL_HEADER )
				return iResult;
			if( (mZipItem.flags & 8) == 8 )
			{
				CLog::Log(LOGERROR,"FileZip: extended local header, not supported!");
				return iResult;
			}
		}

		if (!InitDecompress())
			return iResult;

		// we have a file - fill the buffer
		char* temp;
		int toRead=0;

		if (isGZ)
		{
			m_ZStream.avail_in = strInput.size();
			m_ZStream.next_in = (Bytef*)strInput.data();
			temp = new char[8192];
			toRead = 8191;
		}
		else
		{
			m_ZStream.avail_in = mZipItem.csize;
			m_ZStream.next_in = (Bytef*)strInput.data()+iPos+LHDR_SIZE+mZipItem.flength+mZipItem.elength;
			// init m_zipitem
			strDest.reserve(mZipItem.usize);
			temp = new char[mZipItem.usize+1];
			toRead = mZipItem.usize;
		}
		int iCurrResult;
		while( (iCurrResult=Read(temp,toRead)) > 0)
		{
			strDest.append(temp,temp+iCurrResult);
			iResult += iCurrResult;
		}
		Close();
		delete[] temp;
		iPos += LHDR_SIZE+mZipItem.flength+mZipItem.elength+mZipItem.csize;
		if (isGZ)
			break;
	}

	return iResult;
}
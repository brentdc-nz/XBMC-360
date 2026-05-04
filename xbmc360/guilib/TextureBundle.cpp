#include "TextureBundle.h"
#include "utils\minilzo.h"
#include "GraphicContext.h"
#include "utils\Log.h"
#include "utils\URIUtils.h"
#include "utils\SingleLock.h"
#include "SkinInfo.h"
#include "GUISettings.h"

// Alignment for sector-aligned I/O
#define ALIGN 512

// XPR magic base value
#define XPR_MAGIC_VALUE (('0' << 24) | ('R' << 16) | ('P' << 8) | 'X')

// Xbox 360 D3D format for linear A8R8G8B8
#define X360_D3DFMT_LIN_A8R8G8B8 0x46

CTextureBundle::CTextureBundle(void)
{
	m_hFile = INVALID_HANDLE_VALUE;
	m_themeBundle = false;
	m_lzoInitialized = false;
	m_PreloadIdx = 0;
	m_LoadIdx = 0;
	m_PreLoadBuffer[0] = m_PreLoadBuffer[1] = NULL;
	memset(&m_Ovl[0], 0, sizeof(OVERLAPPED));
	memset(&m_Ovl[1], 0, sizeof(OVERLAPPED));
	memset(&m_TimeStamp, 0, sizeof(m_TimeStamp));
}

CTextureBundle::~CTextureBundle(void)
{
	Cleanup();
}

DWORD CTextureBundle::FromBE32(DWORD v)
{
	// Xbox 360 is big-endian, so on 360 this is a no-op.
	// The data is stored in big-endian by the build tool.
#ifdef _XBOX
	return v; // Already native endian on 360
#else
	// For testing on PC (little-endian), swap bytes
	return ((v & 0xFF) << 24) | ((v & 0xFF00) << 8) |
				 ((v & 0xFF0000) >> 8) | ((v & 0xFF000000) >> 24);
#endif
}

CStdString CTextureBundle::Normalize(const CStdString& name)
{
	CStdString newName(name);
	newName.MakeLower();
	newName.Replace('/', '\\');
	return newName;
}

bool CTextureBundle::OpenBundle()
{
	if (m_hFile != INVALID_HANDLE_VALUE)
		Cleanup();

	CStdString strPath;

	if (m_themeBundle)
	{
		CStdString themeXPR = g_guiSettings.GetString("lookandfeel.skintheme");
		if (!themeXPR.IsEmpty() && themeXPR.CompareNoCase("SKINDEFAULT"))
		{
			strPath = URIUtils::AddFileToFolder(g_graphicsContext.GetMediaDir(), "media");
			strPath = URIUtils::AddFileToFolder(strPath, themeXPR);
		}
		else
			return false;
	}
	else
	{
		if (g_graphicsContext.GetMediaDir().IsEmpty())
			return false;
		strPath = URIUtils::AddFileToFolder(g_graphicsContext.GetMediaDir(), "media\\Textures.xpr");
	}

	if (GetFileAttributes(strPath.c_str()) == (DWORD)-1)
		return false;

	m_hFile = CreateFile(strPath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
											 NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL);
	if (m_hFile == INVALID_HANDLE_VALUE)
	{
		CLog::Log(LOGERROR, "TextureBundle: Unable to open %s: 0x%x", strPath.c_str(), GetLastError());
		return false;
	}

	// Read first sector containing XPR header using overlapped I/O
	BYTE headerBuf[ALIGN];
	OVERLAPPED ovl;
	memset(&ovl, 0, sizeof(ovl));
	DWORD bytesRead;
	if (!ReadFile(m_hFile, headerBuf, ALIGN, &bytesRead, &ovl) && GetLastError() != ERROR_IO_PENDING)
	{
		CLog::Log(LOGERROR, "TextureBundle: Failed to read header from %s", strPath.c_str());
		goto LoadError;
	}
	if (!GetOverlappedResult(m_hFile, &ovl, &bytesRead, TRUE) || bytesRead < sizeof(XPR_HEADER))
	{
		CLog::Log(LOGERROR, "TextureBundle: Failed to read header from %s", strPath.c_str());
		goto LoadError;
	}

	{
		XPR_HEADER* pHeader = (XPR_HEADER*)headerBuf;

		// Headers are big-endian from the build tool
		DWORD magic = FromBE32(pHeader->dwMagic);
		DWORD headerSize = FromBE32(pHeader->dwHeaderSize);

		int version = (magic >> 24) - '0';
		magic -= version << 24;
		version &= 0x0f;

		if (magic != XPR_MAGIC_VALUE || version < 5)
		{
			CLog::Log(LOGERROR, "TextureBundle: Invalid XPR version %d in %s (expected >= 5)", version, strPath.c_str());
			goto LoadError;
		}

		// Read remaining header data if it extends beyond first sector
		DWORD totalHeaderBytes = headerSize;
		BYTE* fullHeader = NULL;

		if (totalHeaderBytes > ALIGN)
		{
			// Allocate sector-aligned buffer for the full header
			DWORD alignedHeaderSize = (totalHeaderBytes + (ALIGN - 1)) & ~(ALIGN - 1);
			fullHeader = (BYTE*)malloc(alignedHeaderSize);
			if (!fullHeader) goto LoadError;

			memcpy(fullHeader, headerBuf, ALIGN);

			OVERLAPPED ovl2;
			memset(&ovl2, 0, sizeof(ovl2));
			ovl2.Offset = ALIGN;
			DWORD remaining = alignedHeaderSize - ALIGN;
			if (!ReadFile(m_hFile, fullHeader + ALIGN, remaining, &bytesRead, &ovl2) && GetLastError() != ERROR_IO_PENDING)
			{
				free(fullHeader);
				goto LoadError;
			}
			if (!GetOverlappedResult(m_hFile, &ovl2, &bytesRead, TRUE) || bytesRead < (totalHeaderBytes - ALIGN))
			{
				free(fullHeader);
				goto LoadError;
			}
		}
		else
		{
			fullHeader = (BYTE*)malloc(totalHeaderBytes);
			if (!fullHeader) goto LoadError;
			memcpy(fullHeader, headerBuf, totalHeaderBytes);
		}

		// Parse file headers (128 bytes each: 116-char name + 3 DWORDs)
		struct DiskFileHeader_t
		{
			char Name[116];
			DWORD Offset;
			DWORD UnpackedSize;
			DWORD PackedSize;
		};

		DiskFileHeader_t* fileHeaders = (DiskFileHeader_t*)(fullHeader + sizeof(XPR_HEADER));
		DWORD numFiles = (headerSize - sizeof(XPR_HEADER)) / sizeof(DiskFileHeader_t);

		for (DWORD i = 0; i < numFiles; i++)
		{
			std::pair<CStdString, FileHeader_t> entry;
			entry.first = Normalize(fileHeaders[i].Name);
			entry.second.Offset = FromBE32(fileHeaders[i].Offset);
			entry.second.UnpackedSize = FromBE32(fileHeaders[i].UnpackedSize);
			entry.second.PackedSize = FromBE32(fileHeaders[i].PackedSize);
			m_FileHeaders.insert(entry);
		}

		free(fullHeader);
	}

	GetFileTime(m_hFile, NULL, NULL, &m_TimeStamp);

	if (!m_lzoInitialized)
	{
		if (lzo_init() != LZO_E_OK)
		{
			CLog::Log(LOGERROR, "TextureBundle: LZO init failed");
			goto LoadError;
		}
		m_lzoInitialized = true;
	}

	CLog::Log(LOGINFO, "TextureBundle: Loaded %d textures from %s", m_FileHeaders.size(), strPath.c_str());

	// Initialize preload state
	m_CurFileHeader[0] = m_FileHeaders.end();
	m_CurFileHeader[1] = m_FileHeaders.end();
	m_PreloadIdx = 0;
	m_LoadIdx = 0;

	return true;

LoadError:
	if (m_hFile != INVALID_HANDLE_VALUE)
		CloseHandle(m_hFile);
	m_hFile = INVALID_HANDLE_VALUE;
	return false;
}

void CTextureBundle::Cleanup()
{
	if (m_hFile != INVALID_HANDLE_VALUE)
		CloseHandle(m_hFile);
	m_hFile = INVALID_HANDLE_VALUE;
	m_FileHeaders.clear();
	for (int i = 0; i < 2; i++)
	{
		free(m_PreLoadBuffer[i]);
		m_PreLoadBuffer[i] = NULL;
		memset(&m_Ovl[i], 0, sizeof(OVERLAPPED));
	}
	m_PreloadIdx = 0;
	m_LoadIdx = 0;
}

bool CTextureBundle::HasFile(const CStdString& Filename)
{
	if (m_hFile == INVALID_HANDLE_VALUE && !OpenBundle())
		return false;

	CStdString name = Normalize(Filename);
	return m_FileHeaders.find(name) != m_FileHeaders.end();
}

void CTextureBundle::GetTexturesFromPath(const CStdString& path, std::vector<CStdString>& textures)
{
	if (path.GetLength() > 1 && path[1] == ':')
		return;

	if (m_hFile == INVALID_HANDLE_VALUE && !OpenBundle())
		return;

	CStdString testPath = Normalize(path);
	if (!URIUtils::HasSlashAtEnd(testPath))
		testPath += "\\";
	int testLength = testPath.GetLength();

	std::map<CStdString, FileHeader_t>::iterator it;
	for (it = m_FileHeaders.begin(); it != m_FileHeaders.end(); it++)
	{
		if (it->first.Left(testLength).Equals(testPath))
			textures.push_back(it->first);
	}
}

void CTextureBundle::GetAllFiles(std::vector<CStdString>& files)
{
	if (m_hFile == INVALID_HANDLE_VALUE && !OpenBundle())
		return;

	std::map<CStdString, FileHeader_t>::iterator it;
	for (it = m_FileHeaders.begin(); it != m_FileHeaders.end(); it++)
		files.push_back(it->first);
}

HRESULT CTextureBundle::LoadFile(const CStdString& Filename, BYTE** ppData, DWORD* pSize)
{
	if (Filename == "-")
		return E_FAIL;

	CStdString name = Normalize(Filename);

	// Check if data is already preloaded in one of the double buffers
	if (m_CurFileHeader[0] != m_FileHeaders.end() && m_CurFileHeader[0]->first == name)
		m_LoadIdx = 0;
	else if (m_CurFileHeader[1] != m_FileHeaders.end() && m_CurFileHeader[1]->first == name)
		m_LoadIdx = 1;
	else
	{
		// Not preloaded - do a synchronous preload now
		m_LoadIdx = m_PreloadIdx;
		if (!PreloadFile(Filename))
			return E_FAIL;
	}

	if (!m_PreLoadBuffer[m_LoadIdx])
		return E_OUTOFMEMORY;

	// Wait for the overlapped read to complete
	DWORD n;
	if (!GetOverlappedResult(m_hFile, &m_Ovl[m_LoadIdx], &n, TRUE) ||
			n < m_CurFileHeader[m_LoadIdx]->second.PackedSize)
	{
		CLog::Log(LOGERROR, "TextureBundle: Read error for %s: 0x%x", Filename.c_str(), GetLastError());
		free(m_PreLoadBuffer[m_LoadIdx]);
		m_PreLoadBuffer[m_LoadIdx] = NULL;
		m_CurFileHeader[m_LoadIdx] = m_FileHeaders.end();
		return E_FAIL;
	}

	// Decompress
	DWORD unpackedSize = m_CurFileHeader[m_LoadIdx]->second.UnpackedSize;
	BYTE* unpackedBuf = (BYTE*)malloc(unpackedSize);
	if (!unpackedBuf)
	{
		free(m_PreLoadBuffer[m_LoadIdx]);
		m_PreLoadBuffer[m_LoadIdx] = NULL;
		m_CurFileHeader[m_LoadIdx] = m_FileHeaders.end();
		return E_OUTOFMEMORY;
	}

	lzo_uint unpackedLen = unpackedSize;
	int lzoResult = lzo1x_decompress_safe(m_PreLoadBuffer[m_LoadIdx],
																				m_CurFileHeader[m_LoadIdx]->second.PackedSize,
																				unpackedBuf, &unpackedLen, NULL);

	// Free the compressed read buffer
	free(m_PreLoadBuffer[m_LoadIdx]);
	m_PreLoadBuffer[m_LoadIdx] = NULL;
	m_CurFileHeader[m_LoadIdx] = m_FileHeaders.end();

	if (lzoResult != LZO_E_OK || unpackedLen != unpackedSize)
	{
		CLog::Log(LOGERROR, "TextureBundle: Decompression error for %s (lzo=%d)", Filename.c_str(), lzoResult);
		free(unpackedBuf);
		return E_FAIL;
	}

	*ppData = unpackedBuf;
	*pSize = unpackedSize;
	return S_OK;
}

bool CTextureBundle::PreloadFile(const CStdString& Filename)
{
	CStdString name = Normalize(Filename);

	// Free any existing buffer in this slot
	if (m_PreLoadBuffer[m_PreloadIdx])
		free(m_PreLoadBuffer[m_PreloadIdx]);
	m_PreLoadBuffer[m_PreloadIdx] = NULL;

	m_CurFileHeader[m_PreloadIdx] = m_FileHeaders.find(name);
	if (m_CurFileHeader[m_PreloadIdx] == m_FileHeaders.end())
		return false;

	// Cancel any pending I/O on this slot
	if (!HasOverlappedIoCompleted(&m_Ovl[m_PreloadIdx]))
	{
		bool flushOther = !HasOverlappedIoCompleted(&m_Ovl[1 - m_PreloadIdx]);
		CancelIo(m_hFile);
		if (flushOther)
		{
			free(m_PreLoadBuffer[1 - m_PreloadIdx]);
			m_PreLoadBuffer[1 - m_PreloadIdx] = NULL;
			m_CurFileHeader[1 - m_PreloadIdx] = m_FileHeaders.end();
		}
	}

	// Allocate sector-aligned read buffer
	DWORD readSize = (m_CurFileHeader[m_PreloadIdx]->second.PackedSize + (ALIGN - 1)) & ~(ALIGN - 1);
	m_PreLoadBuffer[m_PreloadIdx] = (BYTE*)malloc(readSize);
	if (!m_PreLoadBuffer[m_PreloadIdx])
	{
		CLog::Log(LOGERROR, "TextureBundle: Out of memory preloading %s", Filename.c_str());
		m_CurFileHeader[m_PreloadIdx] = m_FileHeaders.end();
		return false;
	}

	// Kick off async read
	memset(&m_Ovl[m_PreloadIdx], 0, sizeof(OVERLAPPED));
	m_Ovl[m_PreloadIdx].Offset = m_CurFileHeader[m_PreloadIdx]->second.Offset;
	m_Ovl[m_PreloadIdx].OffsetHigh = 0;

	DWORD n;
	if (!ReadFile(m_hFile, m_PreLoadBuffer[m_PreloadIdx], readSize, &n, &m_Ovl[m_PreloadIdx]) &&
			GetLastError() != ERROR_IO_PENDING)
	{
		CLog::Log(LOGERROR, "TextureBundle: Error preloading %s: 0x%x", Filename.c_str(), GetLastError());
		free(m_PreLoadBuffer[m_PreloadIdx]);
		m_PreLoadBuffer[m_PreloadIdx] = NULL;
		m_CurFileHeader[m_PreloadIdx] = m_FileHeaders.end();
		return false;
	}

	// Flip to other slot for next preload
	m_PreloadIdx = 1 - m_PreloadIdx;
	return true;
}

HRESULT CTextureBundle::CreateTextureFromData(LPDIRECT3DDEVICE9 pDevice,
																							const TextureInfo360& info,
																							const BYTE* pixelData,
																							LPDIRECT3DTEXTURE9* ppTexture)
{
	// Create a D3D texture matching the stored dimensions
	HRESULT hr = D3DXCreateTexture(pDevice, info.Width, info.Height, 1, 0,
																 D3DFMT_LIN_A8R8G8B8, D3DPOOL_MANAGED, ppTexture);
	if (FAILED(hr))
	{
		CLog::Log(LOGERROR, "TextureBundle: D3DXCreateTexture failed: 0x%x", hr);
		return hr;
	}

	// Lock and copy pixel data directly - it's already in the right format and endianness
	D3DLOCKED_RECT lr;
	hr = (*ppTexture)->LockRect(0, &lr, NULL, 0);
	if (FAILED(hr))
	{
		(*ppTexture)->Release();
		*ppTexture = NULL;
		return hr;
	}

	// Copy row by row respecting pitch differences
	const BYTE* src = pixelData;
	BYTE* dst = (BYTE*)lr.pBits;
	DWORD copyWidth = info.Width * 4; // bytes per row of actual pixels

	for (DWORD y = 0; y < info.Height; y++)
	{
		memcpy(dst, src, copyWidth);
		src += info.Pitch;
		dst += lr.Pitch;
	}

	(*ppTexture)->UnlockRect(0);
	return S_OK;
}

HRESULT CTextureBundle::LoadTexture(LPDIRECT3DDEVICE9 pDevice, const CStdString& Filename,
																		D3DXIMAGE_INFO* pInfo, LPDIRECT3DTEXTURE9* ppTexture)
{
	*ppTexture = NULL;

	BYTE* data = NULL;
	DWORD dataSize = 0;
	HRESULT hr = LoadFile(Filename, &data, &dataSize);
	if (FAILED(hr))
		return hr;

	// Parse the file info header (big-endian)
	if (dataSize < sizeof(FileInfo360))
	{
		free(data);
		return E_FAIL;
	}

	FileInfo360* pFileInfo = (FileInfo360*)data;
	DWORD nImages = FromBE32(pFileInfo->nImages);

	if (nImages == 0)
	{
		free(data);
		return E_FAIL;
	}

	// Parse first texture info
	if (dataSize < sizeof(FileInfo360) + sizeof(TextureInfo360))
	{
		free(data);
		return E_FAIL;
	}

	TextureInfo360* pTexInfo = (TextureInfo360*)(data + sizeof(FileInfo360));
	TextureInfo360 texInfo;
	texInfo.Width = FromBE32(pTexInfo->Width);
	texInfo.Height = FromBE32(pTexInfo->Height);
	texInfo.Format = FromBE32(pTexInfo->Format);
	texInfo.Pitch = FromBE32(pTexInfo->Pitch);
	texInfo.DataOffset = FromBE32(pTexInfo->DataOffset);
	texInfo.DataSize = FromBE32(pTexInfo->DataSize);
	texInfo.Flags = FromBE32(pTexInfo->Flags);
	texInfo.Delay = FromBE32(pTexInfo->Delay);

	// Validate data offset
	if (texInfo.DataOffset + texInfo.DataSize > dataSize)
	{
		CLog::Log(LOGERROR, "TextureBundle: Data offset out of bounds for %s", Filename.c_str());
		free(data);
		return E_FAIL;
	}

	const BYTE* pixelData = data + texInfo.DataOffset;

	CSingleLock lock(g_graphicsContext);
	hr = CreateTextureFromData(pDevice, texInfo, pixelData, ppTexture);

	if (SUCCEEDED(hr) && pInfo)
	{
		pInfo->Width = texInfo.Width;
		pInfo->Height = texInfo.Height;
		pInfo->Depth = 0;
		pInfo->MipLevels = 1;
		pInfo->Format = D3DFMT_LIN_A8R8G8B8;
	}

	free(data);
	return hr;
}

HRESULT CTextureBundle::LoadAnim(LPDIRECT3DDEVICE9 pDevice, const CStdString& Filename,
																 D3DXIMAGE_INFO* pInfo,
																 std::vector<LPDIRECT3DTEXTURE9>& textures,
																 std::vector<int>& delays, int& loops)
{
	textures.clear();
	delays.clear();
	loops = 0;

	BYTE* data = NULL;
	DWORD dataSize = 0;
	HRESULT hr = LoadFile(Filename, &data, &dataSize);
	if (FAILED(hr))
		return hr;

	if (dataSize < sizeof(FileInfo360))
	{
		free(data);
		return E_FAIL;
	}

	FileInfo360* pFileInfo = (FileInfo360*)data;
	DWORD nImages = FromBE32(pFileInfo->nImages);
	loops = (int)FromBE32(pFileInfo->nLoops);

	if (nImages == 0 || dataSize < sizeof(FileInfo360) + nImages * sizeof(TextureInfo360))
	{
		free(data);
		return E_FAIL;
	}

	CSingleLock lock(g_graphicsContext);

	TextureInfo360* pTexInfos = (TextureInfo360*)(data + sizeof(FileInfo360));

	for (DWORD i = 0; i < nImages; i++)
	{
		TextureInfo360 texInfo;
		texInfo.Width = FromBE32(pTexInfos[i].Width);
		texInfo.Height = FromBE32(pTexInfos[i].Height);
		texInfo.Format = FromBE32(pTexInfos[i].Format);
		texInfo.Pitch = FromBE32(pTexInfos[i].Pitch);
		texInfo.DataOffset = FromBE32(pTexInfos[i].DataOffset);
		texInfo.DataSize = FromBE32(pTexInfos[i].DataSize);
		texInfo.Flags = FromBE32(pTexInfos[i].Flags);
		texInfo.Delay = FromBE32(pTexInfos[i].Delay);

		if (texInfo.DataOffset + texInfo.DataSize > dataSize)
		{
			CLog::Log(LOGERROR, "TextureBundle: Frame %d data offset out of bounds for %s", i, Filename.c_str());
			// Release already-created textures
			for (size_t j = 0; j < textures.size(); j++)
				textures[j]->Release();
			textures.clear();
			delays.clear();
			free(data);
			return E_FAIL;
		}

		LPDIRECT3DTEXTURE9 pTexture = NULL;
		hr = CreateTextureFromData(pDevice, texInfo, data + texInfo.DataOffset, &pTexture);
		if (FAILED(hr))
		{
			for (size_t j = 0; j < textures.size(); j++)
				textures[j]->Release();
			textures.clear();
			delays.clear();
			free(data);
			return hr;
		}

		textures.push_back(pTexture);
		delays.push_back(texInfo.Delay);

		// Fill pInfo from first frame
		if (i == 0 && pInfo)
		{
			pInfo->Width = texInfo.Width;
			pInfo->Height = texInfo.Height;
			pInfo->Depth = 0;
			pInfo->MipLevels = 1;
			pInfo->Format = D3DFMT_LIN_A8R8G8B8;
		}
	}

	free(data);
	return S_OK;
}

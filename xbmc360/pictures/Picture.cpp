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

// =============================================================================
// Xbox 360 port of xbmc4xbox CPicture.
//
// Load() uses CJpegIO for JPEG fast-path, D3DXCreateTextureFromFileInMemoryEx
// as fallback for PNG/BMP/etc.
// =============================================================================

#include "Picture.h"
#include "filesystem\File.h"
#include "filesystem\Directory.h"
#include "utils\URIUtils.h"
#include "utils\Log.h"
#include "utils\JpegIO.h"
#include "guilib\GraphicContext.h"
#include "AdvancedSettings.h"
#include "GUISettings.h"

#include <algorithm>

using namespace XFILE;

// Helper: ensure the full directory tree for a file path exists
static void EnsureDirectoryExists(const CStdString& filePath)
{
	CStdString strDir;
	URIUtils::GetDirectory(filePath, strDir);
	if (!strDir.IsEmpty() && !CDirectory::Exists(strDir))
	{
		CStdString normalized(strDir);
		normalized.Replace("/", "\\");
		int pos = normalized.Find('\\');
		while (pos != -1)
		{
			CStdString partial = normalized.Left(pos);
			if (!partial.IsEmpty() && partial.GetLength() > 2)
			{
				if (!CDirectory::Exists(partial))
					CDirectory::Create(partial);
			}
			pos = normalized.Find('\\', pos + 1);
		}
		if (!CDirectory::Exists(normalized))
			CDirectory::Create(normalized);
	}
}

// Helper: use D3DX to decode any image format from memory, resize, then
// re-encode as JPEG via CJpegIO.  This is the 360 equivalent of the OG's
// DllImageLib (CxImage) fallback path.
static bool D3DXCreateThumbnail(const unsigned char* fileData, unsigned int fileSize,
                                const CStdString& destFile, int width, int height)
{
	LPDIRECT3DDEVICE9 pDevice = g_graphicsContext.Get3DDevice();
	if (!pDevice || !fileData || fileSize == 0)
		return false;

	LPDIRECT3DTEXTURE9 pTexture = NULL;
	D3DXIMAGE_INFO imgInfo;
	HRESULT hr = D3DXCreateTextureFromFileInMemoryEx(
		pDevice, fileData, fileSize,
		width, height, 1, 0,
		D3DFMT_LIN_A8R8G8B8, D3DPOOL_MANAGED,
		D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR,
		0, &imgInfo, NULL, &pTexture);
	if (FAILED(hr) || !pTexture)
	{
		CLog::Log(LOGERROR, "%s - D3DX decode failed (0x%08X)", __FUNCTION__, hr);
		return false;
	}

	D3DLOCKED_RECT lr;
	hr = pTexture->LockRect(0, &lr, NULL, 0);
	if (FAILED(hr))
	{
		pTexture->Release();
		return false;
	}

	// Get actual texture dimensions (D3DX may pad to POT)
	D3DSURFACE_DESC desc;
	pTexture->GetLevelDesc(0, &desc);
	unsigned int texW = (std::min)((unsigned int)width,  desc.Width);
	unsigned int texH = (std::min)((unsigned int)height, desc.Height);

	CJpegIO jpegImage;
	bool ret = jpegImage.CreateThumbnailFromSurface(
		(unsigned char*)lr.pBits, texW, texH,
		XB_FMT_A8R8G8B8, lr.Pitch, destFile);

	pTexture->UnlockRect(0);
	pTexture->Release();
	return ret;
}

CPicture::CPicture(void)
{
	ZeroMemory(&m_info, sizeof(ImageInfo));
}

CPicture::~CPicture(void)
{
}

LPDIRECT3DTEXTURE9 CPicture::Load(const CStdString& file, int width, int height)
{
	// JPEG fast-path via libjpeg-turbo
	if (URIUtils::GetExtension(file).Equals(".jpg") || URIUtils::GetExtension(file).Equals(".tbn")
	    || URIUtils::GetExtension(file).Equals(".jpeg"))
	{
		CJpegIO jpegImage;
		if (jpegImage.Open(file, width, height))
		{
			if (jpegImage.OrgWidth() == 0 || jpegImage.OrgHeight() == 0)
				return NULL;

			memset(&m_info, 0, sizeof(ImageInfo));
			m_info.originalwidth = jpegImage.OrgWidth();
			m_info.originalheight = jpegImage.OrgHeight();
			m_info.width = jpegImage.Width();
			m_info.height = jpegImage.Height();

			LPDIRECT3DTEXTURE9 pTexture = NULL;
			LPDIRECT3DDEVICE9 pDevice = g_graphicsContext.Get3DDevice();
			if (!pDevice)
				return NULL;

			pDevice->CreateTexture(
				((m_info.width + 3) / 4) * 4,
				((m_info.height + 3) / 4) * 4,
				1, 0, D3DFMT_LIN_A8R8G8B8, D3DPOOL_MANAGED, &pTexture, NULL);
			if (pTexture)
			{
				D3DLOCKED_RECT lr;
				if (D3D_OK == pTexture->LockRect(0, &lr, NULL, 0))
				{
					DWORD destPitch = lr.Pitch;
					BYTE *pixels = (BYTE *)lr.pBits;
					bool ret = jpegImage.Decode(pixels, destPitch, XB_FMT_A8R8G8B8);
					pTexture->UnlockRect(0);
					if (ret)
						return pTexture;
				}
				pTexture->Release();
			}
			else
			{
				CLog::Log(LOGERROR, "%s - failed to create texture while loading image %s", __FUNCTION__, file.c_str());
			}
			return NULL;
		}
		// JpegIO failed (e.g. EXIF rotation) - fall through to D3DX loader
	}

	// Fallback: load entire file into memory, then use D3DXCreateTextureFromFileInMemoryEx
	CFile cfile;
	if (!cfile.Open(file, READ_TRUNCATED))
	{
		CLog::Log(LOGERROR, "%s - failed to open %s", __FUNCTION__, file.c_str());
		return NULL;
	}

	unsigned int filesize = (unsigned int)cfile.GetLength();
	if (filesize == 0)
		filesize = 64 * 1024; // unknown size, start with 64k

	unsigned char* buffer = NULL;
	unsigned int totalRead = 0;
	unsigned int bufSize = filesize + 1;
	buffer = (unsigned char*)malloc(bufSize);
	if (!buffer)
	{
		cfile.Close();
		return NULL;
	}

	while (true)
	{
		if (totalRead >= bufSize)
		{
			bufSize *= 2;
			unsigned char* newBuf = (unsigned char*)realloc(buffer, bufSize);
			if (!newBuf)
			{
				free(buffer);
				cfile.Close();
				return NULL;
			}
			buffer = newBuf;
		}
		unsigned int bytesRead = cfile.Read(buffer + totalRead, bufSize - totalRead);
		if (!bytesRead)
			break;
		totalRead += bytesRead;
	}
	cfile.Close();

	if (totalRead == 0)
	{
		free(buffer);
		return NULL;
	}

	LPDIRECT3DTEXTURE9 pTexture = NULL;
	LPDIRECT3DDEVICE9 pDevice = g_graphicsContext.Get3DDevice();
	D3DXIMAGE_INFO imgInfo;

	if (pDevice)
	{
		HRESULT hr = D3DXCreateTextureFromFileInMemoryEx(
			pDevice, buffer, totalRead,
			width, height, 1, 0,
			D3DFMT_LIN_A8R8G8B8, D3DPOOL_MANAGED,
			D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR,
			0, &imgInfo, NULL, &pTexture);
		if (SUCCEEDED(hr) && pTexture)
		{
			memset(&m_info, 0, sizeof(ImageInfo));
			m_info.width = imgInfo.Width;
			m_info.height = imgInfo.Height;
			m_info.originalwidth = imgInfo.Width;
			m_info.originalheight = imgInfo.Height;
		}
		else
		{
			CLog::Log(LOGERROR, "%s - D3DXCreateTextureFromFileInMemoryEx failed for %s (0x%08X)", __FUNCTION__, file.c_str(), hr);
			pTexture = NULL;
		}
	}

	free(buffer);
	return pTexture;
}

bool CPicture::CreateThumbnail(const CStdString& file, const CStdString& thumbFile, bool checkExistence /* = false */)
{
	// Don't create the thumb if it already exists
	if (checkExistence && CFile::Exists(thumbFile))
		return true;

	return CacheImage(file, thumbFile, g_advancedSettings.m_thumbSize, g_advancedSettings.m_thumbSize);
}

bool CPicture::CacheThumb(const CStdString& sourceUrl, const CStdString& destFile)
{
	return CacheImage(sourceUrl, destFile, g_advancedSettings.m_thumbSize, g_advancedSettings.m_thumbSize);
}

bool CPicture::CacheFanart(const CStdString& sourceUrl, const CStdString& destFile)
{
	int height = g_advancedSettings.m_fanartHeight;
	// Assume 16:9 size
	int width = height * 16 / 9;

	return CacheImage(sourceUrl, destFile, width, height);
}

bool CPicture::CacheImage(const CStdString& sourceUrl, const CStdString& destFile, int width, int height)
{
	if (width > 0 && height > 0)
	{
		CLog::Log(LOGINFO, "Caching image from: %s to %s with width %i and height %i", sourceUrl.c_str(), destFile.c_str(), width, height);
		EnsureDirectoryExists(destFile);

		// Try CJpegIO fast path for JPEG/TBN (decode + libjpeg downscale + re-encode)
		bool ret = false;
		if (URIUtils::GetExtension(sourceUrl).Equals(".jpg") || URIUtils::GetExtension(sourceUrl).Equals(".tbn")
		    || URIUtils::GetExtension(sourceUrl).Equals(".jpeg"))
		{
			CJpegIO jpegImage;
			ret = jpegImage.CreateThumbnail(sourceUrl, destFile, width, height);
		}

		if (!ret)
		{
			// Fallback: D3DX decode (handles PNG/BMP/GIF/JPEG) + resize + re-encode as JPEG
			CFile cfile;
			if (cfile.Open(sourceUrl, READ_TRUNCATED))
			{
				unsigned int filesize = (unsigned int)cfile.GetLength();
				unsigned int chunksize = filesize ? (filesize + 1) : 65536U;
				unsigned int bufSize = chunksize;
				unsigned char* buffer = (unsigned char*)malloc(bufSize);
				unsigned int totalRead = 0;
				if (buffer)
				{
					while (true)
					{
						if (totalRead >= bufSize)
						{
							bufSize *= 2;
							unsigned char* newBuf = (unsigned char*)realloc(buffer, bufSize);
							if (!newBuf) { free(buffer); buffer = NULL; break; }
							buffer = newBuf;
						}
						unsigned int bytesRead = cfile.Read(buffer + totalRead, bufSize - totalRead);
						if (!bytesRead) break;
						totalRead += bytesRead;
					}
				}
				cfile.Close();

				if (buffer && totalRead > 0)
				{
					ret = D3DXCreateThumbnail(buffer, totalRead, destFile, width, height);
					free(buffer);
				}
			}
		}

		if (!ret)
		{
			CLog::Log(LOGERROR, "%s Unable to create thumbnail %s from %s", __FUNCTION__, destFile.c_str(), sourceUrl.c_str());
			return false;
		}
	}
	else
	{
		CLog::Log(LOGINFO, "Caching image from: %s to %s", sourceUrl.c_str(), destFile.c_str());
		EnsureDirectoryExists(destFile);

		if (!CFile::Cache(sourceUrl, destFile))
		{
			CLog::Log(LOGERROR, "CPicture: failed to cache %s to %s", sourceUrl.c_str(), destFile.c_str());
			return false;
		}
	}
	return true;
}

bool CPicture::CreateThumbnailFromMemory(const unsigned char* buffer, int bufSize, const CStdString& extension, const CStdString& thumbFile)
{
	CLog::Log(LOGINFO, "Creating album thumb from memory: %s", thumbFile.c_str());
	EnsureDirectoryExists(thumbFile);

	// Try CJpegIO fast path for JPEG data
	if (extension.Equals("jpg") || extension.Equals("tbn") || extension.Equals("jpeg"))
	{
		CJpegIO jpegImage;
		if (jpegImage.CreateThumbnailFromMemory((unsigned char*)buffer, bufSize, thumbFile, g_advancedSettings.m_thumbSize, g_advancedSettings.m_thumbSize))
			return true;
	}

	// Fallback: D3DX decode (handles PNG/BMP/GIF/JPEG) + re-encode as JPEG thumbnail
	if (D3DXCreateThumbnail(buffer, bufSize, thumbFile, g_advancedSettings.m_thumbSize, g_advancedSettings.m_thumbSize))
		return true;

	CLog::Log(LOGERROR, "%s: failed for fileType: %s", __FUNCTION__, extension.c_str());
	return false;
}

bool CPicture::CreateThumbnailFromSurface(const unsigned char* buffer, int width, int height, int stride, const CStdString& thumbFile)
{
	EnsureDirectoryExists(thumbFile);

	CJpegIO jpegImage;
	if (jpegImage.CreateThumbnailFromSurface((BYTE *)buffer, width, height, XB_FMT_A8R8G8B8, stride, thumbFile))
		return true;

	CLog::Log(LOGERROR, "%s: failed for %s", __FUNCTION__, thumbFile.c_str());
	return false;
}

void CPicture::CreateFolderThumb(const CStdString* /*thumbs*/, const CStdString& folderThumb)
{
	// TODO: Port when image subsystem is ported (4-tile folder mosaic)
	CLog::Log(LOGWARNING, "CPicture::CreateFolderThumb: not implemented (%s)", folderThumb.c_str());
}

bool CPicture::CacheSkinImage(const CStdString& /*srcFile*/, const CStdString& /*destFile*/)
{
	// TODO: Port when image subsystem is ported
	return false;
}

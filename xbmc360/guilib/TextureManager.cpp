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

#include "TextureManager.h"
#include "GraphicContext.h"
#include "utils\Log.h"
#include "utils\SingleLock.h"
#include "URL.h"
#include "utils\URIUtils.h"
#include "filesystem\Directory.h"
#include "filesystem\File.h"
#include "SkinInfo.h"

CGUITextureManager g_TextureManager;

CTexture::CTexture(int width, int height, int loops/*, LPDIRECT3DPALETTE8 palette*/, bool packed, bool texCoordsArePixels)
{
  m_width = width;
  m_height = height;
  m_loops = loops;
//  m_palette = palette;
  m_texWidth = 0;
  m_texHeight = 0;
#ifdef HAS_XBOX_D3D
  m_texCoordsArePixels = texCoordsArePixels;
  if (m_palette)
    m_palette->AddRef();
#else
  m_texCoordsArePixels = false;
#endif
  m_packed = packed;
};

unsigned int CTexture::size() const
{
  return m_textures.size();
}

void CTexture::Add(LPDIRECT3DTEXTURE9 texture, int delay)
{
  if (!texture)
    return;

  m_textures.push_back(texture);
  m_delays.push_back(delay ? delay * 2 : 100);
  D3DSURFACE_DESC desc;
  if (D3D_OK == texture->GetLevelDesc(0, &desc))
  {
    m_texWidth = desc.Width;
    m_texHeight = desc.Height;
#if 1//def HAS_XBOX_D3D
	if(desc.Format == D3DFMT_LIN_A8R8G8B8)
    m_texCoordsArePixels = true;
#endif
  }
}

void CTexture::Set(LPDIRECT3DTEXTURE9 texture, int width, int height)
{
  assert(!m_textures.size()); // Don't try and set a texture if we already have one!
  m_width = width;
  m_height = height;
  Add(texture, 100);
}

void CTexture::Free()
{
  CSingleLock lock(g_graphicsContext);
  for (unsigned int i = 0; i < m_textures.size(); i++)
  {
    if (m_packed)
    {
#ifdef HAS_XBOX_D3D
      m_textures[i]->BlockUntilNotBusy();
      void* Data = (void*)(*(DWORD*)(((char*)m_textures[i]) + sizeof(D3DTexture)));
      if (Data)
        XPhysicalFree(Data);
      delete [] m_textures[i];
#else
      m_textures[i]->Release();
#endif
    }
    else
      m_textures[i]->Release();
  }
  m_textures.clear();
  m_delays.clear();
  // Note that in SDL and Win32 we already convert the paletted textures into normal textures,
  // so there's no chance of having m_palette as a real palette
#ifdef HAS_XBOX_D3D
  if (m_palette)
  {
    if (m_packed)
    {
      if ((m_palette->Common & D3DCOMMON_REFCOUNT_MASK) > 1)
        m_palette->Release();
      else
        delete m_palette;
    }
    else
      m_palette->Release();
  }
#endif
//  m_palette = NULL;

  Reset();
}

//-----------------------------------------------------------------------------

CTextureMap::CTextureMap()
{
  m_textureName = "";
  m_referenceCount = 0;
  m_memUsage = 0;
}

CTextureMap::CTextureMap(const CStdString& textureName, int width, int height, int loops/*, LPDIRECT3DPALETTE8 palette*/, bool packed)
: m_texture(width, height, loops/*, palette*/, packed)
{
  m_textureName = textureName;
  m_referenceCount = 0;
  m_memUsage = 0;
}

CTextureMap::~CTextureMap()
{
  FreeTexture();
}

void CTextureMap::Dump() const
{
  if (!m_referenceCount)
    return; // Nothing to see here

  CStdString strLog;
  strLog.Format("  texture:%s has %i frames %i refcount\n", m_textureName.c_str(), m_texture.m_textures.size(), m_referenceCount);
  OutputDebugString(strLog.c_str());
}

const CStdString& CTextureMap::GetName() const
{
  return m_textureName;
}

void CTextureMap::Add(LPDIRECT3DTEXTURE9 pTexture, int delay)
{
#ifdef HAS_SDL_OPENGL
  CGLTexture *glTexture = new CGLTexture(pTexture, false);
  m_texture.Add(glTexture, delay);
#else
  m_texture.Add(pTexture, delay);
#endif

  D3DSURFACE_DESC desc;
  if (pTexture && D3D_OK == pTexture->GetLevelDesc(0, &desc))
  //  m_memUsage += desc.Size;
  int idick = 0;
}

bool CTextureMap::Release()
{
  if (!m_texture.m_textures.size()) return true;
  if (!m_referenceCount) return true;

  m_referenceCount--;
  if (!m_referenceCount)
  {
    FreeTexture();
    return true;
  }
  return false;
}

const CTexture &CTextureMap::GetTexture()
{
  m_referenceCount++;
  return m_texture;
}

void CTextureMap::Flush()
{
  if (!m_referenceCount)
    FreeTexture();
}

void CTextureMap::FreeTexture()
{
  m_texture.Free();
}

unsigned int CTextureMap::GetMemoryUsage() const
{
  return m_memUsage;
}

bool CTextureMap::IsEmpty() const
{
  return m_texture.m_textures.size() == 0;
}

//-----------------------------------------------------------------------------

CGUITextureManager::CGUITextureManager(void)
{
	m_strMediaDir = "";
}

CGUITextureManager::~CGUITextureManager(void)
{
}

int CGUITextureManager::Load(const CStdString& strTextureName, bool checkBundleOnly /*= false */)
{
	// First check of texture exists...
	for (int i=0; i < (int)m_vecTextures.size(); ++i)
	{
		CTextureMap *pMap = m_vecTextures[i];
		if (pMap->GetName() == strTextureName)
		{
			return 1;
		}
	}
  
	LPDIRECT3DTEXTURE9 pTexture;
	
	CStdString strPath = g_SkinInfo.GetSkinPath("media\\" + strTextureName);

	if (strTextureName.c_str()[1] == ':')
		strPath=strTextureName;

	// Normal picture
	D3DXIMAGE_INFO info;
	memset(&info, 0, sizeof(D3DXIMAGE_INFO)); // Stop compiler warning

	if(D3DXCreateTextureFromFileEx(g_graphicsContext.Get3DDevice(), strPath.c_str(),
	   D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 1, 0, D3DFMT_LIN_A8R8G8B8, D3DPOOL_MANAGED,
       D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0, &info, NULL, &pTexture) != D3D_OK)
	{
		CLog::Log(LOGWARNING, "Texture manager unable to find file: %s \n", strPath.c_str());
		return NULL;
	}

	CTextureMap* pMap = new CTextureMap(strTextureName, info.Width, info.Height, 0, false);

	pMap->Add(pTexture, 0);

	m_vecTextures.push_back(pMap);

	return TRUE;
}

static const CTexture emptyTexture;

const CTexture &CGUITextureManager::GetTexture(const CStdString& strTextureName)
{
  //  CLog::Log(LOGINFO, " refcount++ for  GetTexture(%s)\n", strTextureName.c_str());
  for (int i = 0; i < (int)m_vecTextures.size(); ++i)
  {
    CTextureMap *pMap = m_vecTextures[i];
    if (pMap->GetName() == strTextureName)
    {
      //CLog::Log(LOGDEBUG, "Total memusage %u", GetMemoryUsage());
      return pMap->GetTexture();
    }
  }
  return emptyTexture;
}

void CGUITextureManager::Flush()
{
	CSingleLock lock(g_graphicsContext);

	ivecTextures i;
	i = m_vecTextures.begin();

	while (i != m_vecTextures.end())
	{
		CTextureMap* pMap = *i;
		pMap->Flush();
		if (pMap->IsEmpty() )
		{
			delete pMap;
			i = m_vecTextures.erase(i);
		}
		else
		{
			i++;
		}
	}
}

void CGUITextureManager::Cleanup()
{
	ivecTextures i;
	i = m_vecTextures.begin();

	while(i != m_vecTextures.end())
	{
		CTextureMap* pMap=*i;
		delete pMap;
		i = m_vecTextures.erase(i);
	}
}

CStdString CGUITextureManager::GetTexturePath(const CStdString &textureName, bool directory /* = false */)
{
	if(CURL::IsFullPath(textureName))
		return textureName;
	else
	{ 
		// Texture doesn't include the full path, so check all fallbacks
		for(vector<CStdString>::iterator it = m_texturePaths.begin(); it != m_texturePaths.end(); ++it)
		{
			CStdString path = URIUtils::AddFileToFolder(it->c_str(), "media");
			path = URIUtils::AddFileToFolder(path, textureName);
			if(directory)
			{
				if(XFILE::CDirectory::Exists(path))
					return path;
			}
			else
			{
				if(XFILE::CFile::Exists(path))
				return path;
			}
		}
	}
	return "";
}

void CGUITextureManager::SetTexturePath(const CStdString &texturePath)
{
	m_texturePaths.clear();
	AddTexturePath(texturePath);
}

void CGUITextureManager::AddTexturePath(const CStdString &texturePath)
{
	if(!texturePath.IsEmpty())
		m_texturePaths.push_back(texturePath);
}

void CGUITextureManager::RemoveTexturePath(const CStdString &texturePath)
{
	for(vector<CStdString>::iterator it = m_texturePaths.begin(); it != m_texturePaths.end(); ++it)
	{
		if(*it == texturePath)
		{
			m_texturePaths.erase(it);
			return;
		}
	}
}

bool CGUITextureManager::CanLoad(const CStdString &texturePath) const
{
	if(texturePath == "-")
		return false;

	if(!CURL::IsFullPath(texturePath))
		return true; // Assume we have it

	// We can't (or shouldn't) be loading from remote paths, so check these
	return URIUtils::IsHD(texturePath);
}
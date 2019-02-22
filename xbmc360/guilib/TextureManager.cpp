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
#include "..\utils\Log.h"
#include "..\utils\SingleLock.h"

CGUITextureManager g_TextureManager;

CTexture::CTexture()
{
	m_iWidth = m_iHeight=0;
}

CTexture::CTexture(LPDIRECT3DTEXTURE9 pTexture,int iWidth, int iHeight)
{
	m_iWidth = iWidth;
	m_iHeight = iHeight;

	m_pTexture = pTexture;
}

CTexture::~CTexture()
{
	FreeTexture();
}

LPDIRECT3DTEXTURE9 CTexture::GetTexture(/*int& iWidth, int& iHeight*/)
{
	if (!m_pTexture) return NULL;

//	iWidth = m_iWidth;
//	iHeight = m_iHeight;

	return m_pTexture;
}

void CTexture::FreeTexture()
{
	if (m_pTexture)
		m_pTexture->Release();
	m_pTexture=NULL;
}

void CTexture::Flush()
{
	FreeTexture();
}


//-----------------------------------------------------------------------------

CTextureMap::CTextureMap()
{
	m_strTextureName = "";
}

CTextureMap::CTextureMap(const CStdString& strTextureName)
{
	m_strTextureName = strTextureName;
}

CTextureMap::~CTextureMap()
{
	for (int i=0; i < (int)m_vecTexures.size(); ++i)
	{
		CTexture* pTexture = m_vecTexures[i];
		delete pTexture;
	}
	m_vecTexures.erase(m_vecTexures.begin(), m_vecTexures.end());
}

const CStdString& CTextureMap:: GetName() const
{
	return m_strTextureName;
}

int CTextureMap::size() const
{
	return  m_vecTexures.size();
}

LPDIRECT3DTEXTURE9 CTextureMap::GetTexture(/*int iPicture, int& iWidth, int& iHeight*/)
{
//	if (iPicture < 0 || iPicture >= (int)m_vecTexures.size()) return NULL;
  
	CTexture* pTexture = m_vecTexures[/*iPicture*/0];
	return pTexture->GetTexture(/*iWidth, iHeight*/);
}

void CTextureMap::Add(CTexture* pTexture)
{
	m_vecTexures.push_back(pTexture);
}

bool CTextureMap::IsEmpty() const //MARTY FIXME 
{
/*	int iRef = 0;
	for (int i = 0; i < (int)m_vecTexures.size(); ++i)
	{
		iRef += m_vecTexures[i]->GetRef();
	}
	return (iRef == 0);
*/
	int iRef = 0;
	for (int i = 0; i < (int)m_vecTexures.size(); ++i)
	{
		iRef ++;
	}
	return (iRef != 0);
}

void CTextureMap::Flush()
{
	for (int i = 0; i < (int)m_vecTexures.size(); ++i)
	{
		m_vecTexures[i]->Flush();
	}
}

//-----------------------------------------------------------------------------

CGUITextureManager::CGUITextureManager(void)
{
	m_strMediaDir = "";
}

CGUITextureManager::~CGUITextureManager(void)
{
}

void CGUITextureManager::SetTexturePath(const CStdString& strMediaPath)
{
	m_strMediaDir = strMediaPath;
}

LPDIRECT3DTEXTURE9 CGUITextureManager::GetTexture(const CStdString& strTextureName/*, int iItem, int& iWidth, int& iHeight*/)
{
	for (int i=0; i < (int)m_vecTextures.size(); ++i)
	{
		CTextureMap *pMap=m_vecTextures[i];
		if (pMap->GetName() == strTextureName)
		{
			return  pMap->GetTexture(/*iItem, iWidth, iHeight*/);
		}
	}
	return NULL;
}

int CGUITextureManager::Load(const CStdString& strTextureName, DWORD dwColorKey)
{
	// first check of texture exists...
	for (int i=0; i < (int)m_vecTextures.size(); ++i)
	{
		CTextureMap *pMap = m_vecTextures[i];
		if (pMap->GetName() == strTextureName)
		{
			return pMap->size();
		}
	}
  
	LPDIRECT3DTEXTURE9 pTexture;
	CStdString strPath = m_strMediaDir;
	strPath+="media\\";
	strPath+=strTextureName;

	if (strTextureName.c_str()[1] == ':')
		strPath=strTextureName;

	// Normal picture
	D3DXIMAGE_INFO info;
#if 0 //FIXME - We should be using this!
	if(D3DXCreateTextureFromFileEx(g_graphicsContext.Get3DDevice(), strPath.c_str(),
	   D3DX_DEFAULT, D3DX_DEFAULT, 1, 0, D3DFMT_LIN_A8R8G8B8, D3DPOOL_MANAGED,
       D3DX_FILTER_NONE , D3DX_FILTER_NONE, dwColorKey, &info, NULL, &pTexture)!=D3D_OK)
	{
		CLog::Log(LOGWARNING, "Texture manager unable to find file: %s \n", strPath.c_str());
		return NULL;
	}
#endif

#if 1
	if(D3DXCreateTextureFromFile(g_graphicsContext.Get3DDevice(), strPath.c_str(), &pTexture)!=D3D_OK)
	{
		CLog::Log(LOGWARNING, "Texture manager unable to find file: %s \n", strPath.c_str());
		return NULL;
	}
#endif

	CTextureMap* pMap = new CTextureMap(strTextureName);
	CTexture* pclsTexture = new CTexture(pTexture, info.Width, info.Height);

	pMap->Add(pclsTexture);
	m_vecTextures.push_back(pMap);

	return TRUE;
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
	while (i != m_vecTextures.end())
	{
		CTextureMap* pMap=*i;
		delete pMap;
		i=m_vecTextures.erase(i);
	}
}

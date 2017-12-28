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

#ifndef GUILIB_TEXTUREMANAGER_H
#define GUILIB_TEXTUREMANAGER_H

#include "..\utils\stdafx.h"
#include "..\utils\stdstring.h"
#include <vector>

using namespace std;

class CTexture
{
public:
    CTexture();
    CTexture(LPDIRECT3DTEXTURE9 pTexture,int iWidth, int iHeight);
	virtual ~CTexture();


    LPDIRECT3DTEXTURE9  GetTexture(/*int& iWidth, int& iHeight*/);
	
	void FreeTexture();
	void Flush(); 

protected:
    LPDIRECT3DTEXTURE9  m_pTexture;
	int					m_iWidth;
	int					m_iHeight;
};

class CTextureMap
{
  public:
    CTextureMap();
    CTextureMap(const CStdString& strTextureName);
    virtual   ~CTextureMap();

	const CStdString&   GetName() const;
    int                 size() const;
    LPDIRECT3DTEXTURE9  GetTexture(/*int iPicture, int& iWidth, int& iHeight*/);
    void                Add(CTexture* pTexture);
	bool				IsEmpty() const;
	void				Flush();

protected:  
    CStdString          m_strTextureName;
    vector<CTexture*>   m_vecTexures;
};

class CGUITextureManager
{
public:
	CGUITextureManager(void);
	virtual ~CGUITextureManager(void);

	void SetTexturePath(const CStdString& strMediaPath);
	LPDIRECT3DTEXTURE9 GetTexture(const CStdString& strTextureName/*, int iItem, int& iWidth, int& iHeight*/);
	int Load(const CStdString& strTextureName, DWORD dwColorKey);

	void Flush();
	void Cleanup();

protected:
	CStdString m_strMediaDir;

	vector<CTextureMap*> m_vecTextures;
	typedef   vector<CTextureMap*>::iterator ivecTextures;
};

extern CGUITextureManager g_TextureManager;

#endif //GUILIB_TEXTUREMANAGER_H
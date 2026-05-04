#ifndef GUILIB_TEXTUREMANAGER_H
#define GUILIB_TEXTUREMANAGER_H

#include "utils\Stdafx.h"
#include "utils\stdstring.h"
#include "TextureBundle.h"
#include <vector>
#include <list>

using namespace std;

class CTexture
{
public:
  CTexture()
  {
    Reset();
  };
  void Reset()
  {
    m_textures.clear();
    m_delays.clear();
//    m_palette = NULL;
    m_width = 0;
    m_height = 0;
    m_loops = 0;
    m_texWidth = 0;
    m_texHeight = 0;
    m_texCoordsArePixels = false;
    m_packed = false;
  };
  CTexture(int width, int height, int loops,/* LPDIRECT3DPALETTE9 palette = NULL,*/ bool packed = false, bool texCoordsArePixels = false);
  void Add(LPDIRECT3DTEXTURE9 texture, int delay);
  void Set(LPDIRECT3DTEXTURE9 texture, int width, int height);
  void Free();
  unsigned int size() const;

  std::vector<LPDIRECT3DTEXTURE9> m_textures;
//  LPDIRECT3DPALETTE9 m_palette;
  std::vector<int> m_delays;
  int m_width;
  int m_height;
  int m_loops;
  int m_texWidth;
  int m_texHeight;
  bool m_texCoordsArePixels;
  bool m_packed;
};

class CTextureMap
{
public:
  CTextureMap();
  virtual ~CTextureMap();

  CTextureMap(const CStdString& textureName, int width, int height, int loops/*, LPDIRECT3DPALETTE9 palette*/, bool packed);
  void Add(LPDIRECT3DTEXTURE9 pTexture, int delay);
  bool Release();

  const CStdString& GetName() const;
  const CTexture &GetTexture();
  void Dump() const;
  unsigned int GetMemoryUsage() const;
  void Flush();
  bool IsEmpty() const;
protected:
  void FreeTexture();

  CStdString m_textureName;
  CTexture m_texture;
  unsigned int m_referenceCount;
  unsigned int m_memUsage;
};

class CGUITextureManager
{
public:
	CGUITextureManager(void);
	virtual ~CGUITextureManager(void);

	void StartPreLoad();
	void PreLoad(const CStdString& strTextureName);
	void EndPreLoad();
	void FlushPreLoad();
	bool HasTexture(const CStdString &textureName, CStdString *path = NULL, int *bundle = NULL, int *size = NULL);
	bool CanLoad(const CStdString &texturePath) const;     // Returns true if the texture manager can load this texture
	int Load(const CStdString& strTextureName, bool checkBundleOnly = false);
	const CTexture &GetTexture(const CStdString& strTextureName);
	void ReleaseTexture(const CStdString& strTextureName);
	void Cleanup();
	void Dump() const;
	unsigned int GetMemoryUsage() const;
	void Flush();
	CStdString GetTexturePath(const CStdString& textureName, bool directory = false);
	void GetBundledTexturesFromPath(const CStdString& path, std::vector<CStdString>& items);

	void AddTexturePath(const CStdString &texturePath);    // Add a new path to the paths to check when loading media
	void SetTexturePath(const CStdString &texturePath);    // Set a single path as the path to check when loading media (clear then add)
	void RemoveTexturePath(const CStdString &texturePath); // Remove a path from the paths to check when loading media
	void OpenBundles();
	void LoadAllBundleTextures();

protected:
	CStdString m_strMediaDir;

	vector<CTextureMap*> m_vecTextures;
	typedef   vector<CTextureMap*>::iterator ivecTextures;

	std::list<CStdString> m_PreLoadNames[2];
	std::list<CStdString>::iterator m_iNextPreload[2];
	std::vector<CStdString> m_texturePaths;

	CTextureBundle m_TexBundle[2]; // [0] = skin bundle, [1] = theme bundle
};

extern CGUITextureManager g_TextureManager;

#endif //GUILIB_TEXTUREMANAGER_H
#ifndef GUILIB_TEXTUREBUNDLE_H
#define GUILIB_TEXTUREBUNDLE_H

#include "utils\Stdafx.h"
#include "utils\stdstring.h"
#include <map>
#include <vector>

class CTextureBundle
{
public:
	CTextureBundle(void);
	~CTextureBundle(void);

	// Open the bundle file and read the file index
	bool OpenBundle();

	// Check if a texture exists in this bundle
	bool HasFile(const CStdString& Filename);

	// Get list of texture files under a path in the bundle
	void GetTexturesFromPath(const CStdString& path, std::vector<CStdString>& textures);

	// Get all texture file names in the bundle
	void GetAllFiles(std::vector<CStdString>& files);

	// Kick off an async read for a texture (double-buffered)
	bool PreloadFile(const CStdString& Filename);

	// Load a single texture from the bundle
	// Returns S_OK on success, sets ppTexture and pInfo
	HRESULT LoadTexture(LPDIRECT3DDEVICE9 pDevice, const CStdString& Filename,
											D3DXIMAGE_INFO* pInfo, LPDIRECT3DTEXTURE9* ppTexture);

	// Load an animated texture (multiple frames)
	// Returns S_OK on success, fills ppTextures and pDelays vectors
	HRESULT LoadAnim(LPDIRECT3DDEVICE9 pDevice, const CStdString& Filename,
									 D3DXIMAGE_INFO* pInfo,
									 std::vector<LPDIRECT3DTEXTURE9>& textures,
									 std::vector<int>& delays, int& loops);

	// Close bundle and free resources
	void Cleanup();

	// Normalize a filename for lookup (public for TextureManager)
	static CStdString Normalize(const CStdString& name);

	// Set whether this is a theme bundle
	void SetThemeBundle(bool themeBundle) { m_themeBundle = themeBundle; }
	bool IsThemeBundle() const { return m_themeBundle; }

private:
	// XPR header
	struct XPR_HEADER
	{
		DWORD dwMagic;
		DWORD dwTotalSize;
		DWORD dwHeaderSize;
	};

	// Per-file entry in the bundle index
	struct FileHeader_t
	{
		DWORD Offset;
		DWORD UnpackedSize;
		DWORD PackedSize;
	};

	// Per-texture metadata (matches build tool's XPR360_TextureInfo)
	struct TextureInfo360
	{
		DWORD Width;
		DWORD Height;
		DWORD Format;
		DWORD Pitch;
		DWORD DataOffset;
		DWORD DataSize;
		DWORD Flags;
		DWORD Delay;
	};

	// Per-file metadata (matches build tool's XPR360_FileInfo)
	struct FileInfo360
	{
		DWORD nImages;
		DWORD nLoops;
		DWORD TotalSize;
	};

	// Load and decompress a file's data from the bundle (uses preload if available)
	HRESULT LoadFile(const CStdString& Filename, BYTE** ppData, DWORD* pSize);

	// Create a D3D texture from pre-converted pixel data
	HRESULT CreateTextureFromData(LPDIRECT3DDEVICE9 pDevice,
																const TextureInfo360& info,
																const BYTE* pixelData,
																LPDIRECT3DTEXTURE9* ppTexture);

	// Byte-swap a DWORD from big-endian to native
	static DWORD FromBE32(DWORD v);

	HANDLE m_hFile;
	std::map<CStdString, FileHeader_t> m_FileHeaders;
	FILETIME m_TimeStamp;
	bool m_themeBundle;
	bool m_lzoInitialized;

	// Double-buffered overlapped I/O preload state
	OVERLAPPED m_Ovl[2];
	BYTE* m_PreLoadBuffer[2];
	std::map<CStdString, FileHeader_t>::iterator m_CurFileHeader[2];
	int m_PreloadIdx;  // Next slot to write into
	int m_LoadIdx;     // Slot to read from
};

#endif // GUILIB_TEXTUREBUNDLE_H

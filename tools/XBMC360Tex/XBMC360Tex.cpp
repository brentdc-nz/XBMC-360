// XBMC360Tex - Xbox 360 Texture Packing Tool
//
// Converts source images (PNG, BMP, TGA, JPG, GIF) into an LZO-compressed
// XPR bundle with textures pre-converted to Xbox 360 D3DFMT_LIN_A8R8G8B8
// format with big-endian pixel data.
//
// Based on xbmc4xbox's XBMCTex tool, adapted for Xbox 360:
// - No P8 paletted textures (not supported on 360 D3D9)
// - All textures stored as linear A8R8G8B8
// - Pixel data byte-swapped for big-endian PowerPC
// - Bundle headers byte-swapped for big-endian
// - Version 5 XPR format to distinguish from OG Xbox bundles

#include "AnimatedGif.h"
#include "Bundler360.h"
#include <stdio.h>
#include <algorithm>
#include "cmdlineargs.h"
#include "Surface.h"

#define XBMC_FILE_SEP '\\'

CBundler360 Bundler;

UINT UncompressedSize;
UINT CompressedSize;
UINT TotalSrcPixels;
UINT TotalDstPixels;

BYTE* imageData = NULL;
DWORD imageSize = 0;

// Per-file info written into the XPR data section
XPR360_FileInfo fileInfo;
XPR360_TextureInfo* textureInfos = NULL;
int textureInfoCount = 0;
int textureInfoCapacity = 0;

void CommitXPR360(const char* Filename)
{
	if (textureInfoCount == 0)
		return;

	// Build the per-file buffer:
	// XPR360_FileInfo + N * XPR360_TextureInfo (all byte-swapped) + pixel data

	// Byte-swap the file info header
	XPR360_FileInfo beFileInfo;
	beFileInfo.nImages = ByteSwap32(fileInfo.nImages);
	beFileInfo.nLoops = ByteSwap32(fileInfo.nLoops);
	beFileInfo.TotalSize = ByteSwap32(fileInfo.TotalSize);

	DWORD infoSize = sizeof(XPR360_FileInfo) + textureInfoCount * sizeof(XPR360_TextureInfo);
	DWORD alignedInfoSize = (infoSize + 127) & ~127; // 128-byte align for pixel data

	// Update DataOffset in each texture info to account for the header
	for (int i = 0; i < textureInfoCount; i++)
		textureInfos[i].DataOffset += alignedInfoSize;

	// Byte-swap all texture infos
	BYTE* beTexInfoBuf = (BYTE*)malloc(textureInfoCount * sizeof(XPR360_TextureInfo));
	for (int i = 0; i < textureInfoCount; i++)
	{
		XPR360_TextureInfo* src = &textureInfos[i];
		XPR360_TextureInfo* dst = (XPR360_TextureInfo*)(beTexInfoBuf + i * sizeof(XPR360_TextureInfo));
		dst->Width = ByteSwap32(src->Width);
		dst->Height = ByteSwap32(src->Height);
		dst->Format = ByteSwap32(src->Format);
		dst->Pitch = ByteSwap32(src->Pitch);
		dst->DataOffset = ByteSwap32(src->DataOffset);
		dst->DataSize = ByteSwap32(src->DataSize);
		dst->Flags = ByteSwap32(src->Flags);
		dst->Delay = ByteSwap32(src->Delay);
	}

	// Assemble the complete buffer
	DWORD totalBufSize = alignedInfoSize + imageSize;
	BYTE* totalBuf = (BYTE*)malloc(totalBufSize);
	memset(totalBuf, 0, totalBufSize);

	BYTE* p = totalBuf;
	memcpy(p, &beFileInfo, sizeof(XPR360_FileInfo));
	p += sizeof(XPR360_FileInfo);
	memcpy(p, beTexInfoBuf, textureInfoCount * sizeof(XPR360_TextureInfo));
	p = totalBuf + alignedInfoSize;
	memcpy(p, imageData, imageSize);

	free(beTexInfoBuf);

	const void* Buffers[1] = { totalBuf };
	DWORD Sizes[1] = { totalBufSize };
	if (!Bundler.AddFile(Filename, 1, Buffers, Sizes))
		printf("ERROR: Unable to compress data (out of memory?)\n");

	free(totalBuf);
	free(imageData);
	imageData = NULL;
	imageSize = 0;
	free(textureInfos);
	textureInfos = NULL;
	textureInfoCount = 0;
	textureInfoCapacity = 0;
}

void AppendImage360(CSurface& surface, DWORD flags, DWORD delay)
{
	// All textures are linear A8R8G8B8 on Xbox 360
	// Pitch = width * 4, no power-of-2 padding needed for linear textures
	// but we align rows to 256 bytes for GPU efficiency on 360
	DWORD pitch = (surface.Info().width * 4 + 255) & ~255;
	DWORD dataSize = pitch * surface.Info().height;
	DWORD alignedSize = (dataSize + 127) & ~127; // 128-byte align for next image

	// Grow texture info array
	if (textureInfoCount >= textureInfoCapacity)
	{
		textureInfoCapacity = textureInfoCapacity ? textureInfoCapacity * 2 : 16;
		textureInfos = (XPR360_TextureInfo*)realloc(textureInfos, textureInfoCapacity * sizeof(XPR360_TextureInfo));
	}

	// Fill texture info (native endian - will be byte-swapped in CommitXPR360)
	XPR360_TextureInfo* info = &textureInfos[textureInfoCount];
	info->Width = surface.Info().width;
	info->Height = surface.Info().height;
	info->Format = X360_D3DFMT_LIN_A8R8G8B8;
	info->Pitch = pitch;
	info->DataOffset = imageSize; // relative to start of pixel data, adjusted later
	info->DataSize = dataSize;
	info->Flags = flags;
	info->Delay = delay;

	// Allocate space for pixel data
	imageData = (BYTE*)realloc(imageData, imageSize + alignedSize);
	memset(imageData + imageSize, 0, alignedSize);

	// Copy pixels with proper pitch, then byte-swap for big-endian
	CSurfaceRect rect;
	if (surface.Lock(&rect))
	{
		BYTE* dst = imageData + imageSize;
		BYTE* src = rect.pBits;
		for (UINT y = 0; y < surface.Info().height; y++)
		{
			memcpy(dst, src, surface.Info().width * 4);
			src += rect.Pitch;
			dst += pitch;
		}
		surface.Unlock();
	}

	// Byte-swap all pixels for big-endian Xbox 360
	ByteSwapPixels32((DWORD*)(imageData + imageSize), alignedSize);

	imageSize += alignedSize;
	CompressedSize += alignedSize;
	textureInfoCount++;
}

void WriteXPR360(const char* Filename, CSurface& surface)
{
	// Single image
	fileInfo.nImages = 1;
	fileInfo.nLoops = 0;
	fileInfo.TotalSize = 0;
	textureInfoCount = 0;

	AppendImage360(surface, 0, 0);

	fileInfo.TotalSize = imageSize;

	CommitXPR360(Filename);
}

// Converts any fully transparent pixels to transparent black for better compression
void FixTransparency(CSurface& surface)
{
	CSurfaceRect rect;
	if (!surface.Lock(&rect))
		return;

	DWORD* pix = (DWORD*)rect.pBits;
	for (UINT y = 0; y < surface.Height(); ++y)
	{
		DWORD* row = (DWORD*)(rect.pBits + y * rect.Pitch);
		for (UINT x = 0; x < surface.Width(); ++x)
		{
			if (!(row[x] & 0xff000000))
				row[x] = 0;
		}
	}

	surface.Unlock();
}

void ConvertFile(const char* Dir, const char* Filename)
{
	char OutFilename[260];
	if (Dir)
		_snprintf(OutFilename, sizeof(OutFilename), "%s\\%s", Dir, Filename);
	else
		_snprintf(OutFilename, sizeof(OutFilename), "%s", Filename);
	OutFilename[sizeof(OutFilename) - 1] = 0;

	printf("%s: ", OutFilename);
	int n = (int)strlen(OutFilename);
	if (n < 40)
		printf("%*c", 40 - n, ' ');

	// Load source image as ARGB
	CSurface srcSurface;
	if (!srcSurface.CreateFromFile(Filename, CSurface::FMT_ARGB))
	{
		printf("Error loading image\n");
		return;
	}

	FixTransparency(srcSurface);

	// Xbox 360: Always use linear A8R8G8B8 (no paletted textures, no swizzling)
	// We use the real image dimensions - no power-of-2 padding needed for linear format
	// but pitch is 256-byte aligned
	UncompressedSize += srcSurface.Info().width * srcSurface.Info().height * 4;
	TotalSrcPixels += srcSurface.Info().width * srcSurface.Info().height;
	TotalDstPixels += srcSurface.Info().width * srcSurface.Info().height;

	printf("LIN_A8R8G8B8 %4dx%-4d (BE)\n", srcSurface.Info().width, srcSurface.Info().height);

	WriteXPR360(OutFilename, srcSurface);
}

void ConvertAnim(const char* Dir, const char* Filename)
{
	char OutFilename[260];
	if (Dir)
		_snprintf(OutFilename, sizeof(OutFilename), "%s\\%s", Dir, Filename);
	else
		_snprintf(OutFilename, sizeof(OutFilename), "%s", Filename);
	OutFilename[sizeof(OutFilename) - 1] = 0;

	printf("%s: ", OutFilename);
	int n = (int)strlen(OutFilename);
	if (n < 40)
		printf("%*c", 40 - n, ' ');

	CAnimatedGifSet Anim;
	int nImages = Anim.LoadGIF(Filename);
	if (!nImages)
	{
		puts("ERROR: Unable to load gif");
		return;
	}
	if (nImages > 65535)
	{
		printf("ERROR: Too many frames (%d > 65535)\n", nImages);
		return;
	}

	printf("%4dx%-4d (%5df) -> ", Anim.FrameWidth, Anim.FrameHeight, nImages);

	// Setup file info
	fileInfo.nImages = nImages;
	fileInfo.nLoops = Anim.nLoops;
	fileInfo.TotalSize = 0;
	textureInfoCount = 0;

	COLOR pal[256];
	memcpy(pal, Anim.m_vecimg[0]->Palette, 256 * sizeof(COLOR));
	for (int i = 0; i < 256; i++)
		pal[i].x = 0xff;
	if (Anim.m_vecimg[0]->Transparency && Anim.m_vecimg[0]->Transparent >= 0)
		memset(&pal[Anim.m_vecimg[0]->Transparent], 0, sizeof(COLOR));

	for (int i = 0; i < nImages; i++)
	{
		printf("%3d%%\b\b\b\b", 100 * i / nImages);

		CAnimatedGif* pGif = Anim.m_vecimg[i];

		// Expand P8 to A8R8G8B8 since Xbox 360 has no palette support
		CSurface surface;
		if (!surface.Create(pGif->Width, pGif->Height, CSurface::FMT_ARGB))
			return;

		CSurfaceRect rect;
		if (!surface.Lock(&rect))
			return;

		for (int y = 0; y < pGif->Height; y++)
		{
			DWORD* dst = (DWORD*)(rect.pBits + y * rect.Pitch);
			BYTE* src = (BYTE*)pGif->Raster + y * pGif->BytesPerRow;
			for (int x = 0; x < pGif->Width; x++)
			{
				COLOR* c = &pal[*src++];
				// Store as A8R8G8B8 in little-endian memory: bytes are B,G,R,A
				*dst++ = (c->x << 24) | (c->r << 16) | (c->g << 8) | c->b;
			}
		}
		surface.Unlock();

		UncompressedSize += pGif->Width * pGif->Height * 4;
		TotalSrcPixels += pGif->Width * pGif->Height;
		TotalDstPixels += pGif->Width * pGif->Height;

		AppendImage360(surface, XPR360_FLAG_ANIM, pGif->Delay);
	}

	fileInfo.TotalSize = imageSize;

	printf("LIN_A8R8G8B8 (BE) %d frames\n", nImages);

	CommitXPR360(OutFilename);
}

bool IsGraphicsFile(const char* strFileName)
{
	int n = (int)strlen(strFileName);
	if (n < 4) return false;
	if (_strnicmp(&strFileName[n - 4], ".png", 4) &&
		_strnicmp(&strFileName[n - 4], ".bmp", 4) &&
		_strnicmp(&strFileName[n - 4], ".tga", 4) &&
		_strnicmp(&strFileName[n - 4], ".jpg", 4))
		return false;
	return true;
}

bool IsGraphicsAnim(const char* strFileName)
{
	int n = (int)strlen(strFileName);
	if (n < 4 || _strnicmp(&strFileName[n - 4], ".gif", 4))
		return false;
	return true;
}

void ConvertDirectory(const char* strFullPath, char* strRelativePath)
{
	if (strFullPath)
		SetCurrentDirectory(strFullPath);

	char strCurrentPath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, strCurrentPath);

	WIN32_FIND_DATAA FindData;
	char Filename[4] = "*.*";
	HANDLE hFind = FindFirstFile(Filename, &FindData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (_strnicmp(FindData.cFileName, ".", 1))
				{
					char strNewFullPath[MAX_PATH];
					char strNewRelativePath[MAX_PATH];
					sprintf(strNewFullPath, "%s%c%s", strCurrentPath, XBMC_FILE_SEP, FindData.cFileName);
					if (strRelativePath)
						sprintf(strNewRelativePath, "%s%c%s", strRelativePath, XBMC_FILE_SEP, FindData.cFileName);
					else
						sprintf(strNewRelativePath, "%s", FindData.cFileName);
					ConvertDirectory(strNewFullPath, strNewRelativePath);
					SetCurrentDirectory(strCurrentPath);
				}
			}
			else
			{
				if (IsGraphicsFile(FindData.cFileName))
					ConvertFile(strRelativePath, FindData.cFileName);
				if (IsGraphicsAnim(FindData.cFileName))
					ConvertAnim(strRelativePath, FindData.cFileName);
			}
		} while (FindNextFile(hFind, &FindData));
		FindClose(hFind);
	}
}

void Usage()
{
	puts("XBMC360Tex - Xbox 360 Texture Bundle Packer");
	puts("");
	puts("Converts skin media images into an LZO-compressed XPR bundle");
	puts("with textures pre-converted to Xbox 360 D3DFMT_LIN_A8R8G8B8.");
	puts("");
	puts("Usage:");
	puts("  -help            Show this screen.");
	puts("  -input <dir>     Input directory. Default: current dir");
	puts("  -output <file>   Output filename. Default: Textures.xpr");
}

int main(int argc, char* argv[])
{
	bool valid = false;
	CmdLineArgs args(argc, (const char**)argv);

	if (args.size() == 1)
	{
		Usage();
		return 1;
	}

	const char* InputDir = NULL;
	const char* OutputFilename = "Textures.xpr";

	for (unsigned int i = 1; i < args.size(); ++i)
	{
		if (!_stricmp(args[i], "-help") || !_stricmp(args[i], "-h") || !_stricmp(args[i], "-?"))
		{
			Usage();
			return 1;
		}
		else if (!_stricmp(args[i], "-input") || !_stricmp(args[i], "-i"))
		{
			InputDir = args[++i];
			valid = true;
		}
		else if (!_stricmp(args[i], "-output") || !_stricmp(args[i], "-o"))
		{
			OutputFilename = args[++i];
			valid = true;
		}
		else
		{
			printf("Unrecognised flag: %s\n", args[i]);
		}
	}

	if (!valid)
	{
		Usage();
		return 1;
	}

	// Initialize D3D for image loading
	if (!g_device.Create())
		return 1;

	char HomeDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, HomeDir);

	Bundler.StartBundle();

	ConvertDirectory(InputDir, NULL);

	SetCurrentDirectory(HomeDir);
	DWORD attr = GetFileAttributes(OutputFilename);
	if (attr != (DWORD)-1 && (attr & FILE_ATTRIBUTE_DIRECTORY))
	{
		SetCurrentDirectory(OutputFilename);
		OutputFilename = "Textures.xpr";
	}

	printf("\nWriting Xbox 360 bundle: %s", OutputFilename);
	int BundleSize = Bundler.WriteBundle(OutputFilename);
	if (BundleSize == -1)
	{
		printf("\nERROR: %08x\n", GetLastError());
		return 1;
	}

	printf("\nUncompressed texture size: %6dkB\nCompressed texture size: %8dkB\nBundle size:             %8dkB\n\nTotal pixels: %u\n",
		(UncompressedSize + 1023) / 1024, (((CompressedSize + 1023) / 1024) + 3) & ~3, (BundleSize + 1023) / 1024,
		TotalDstPixels);

	return 0;
}

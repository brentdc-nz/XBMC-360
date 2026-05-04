# XBMC360Tex - Xbox 360 Texture Bundle Packer

## Overview

XBMC360Tex converts skin media images (PNG, BMP, TGA, JPG, GIF) into a single
LZO-compressed XPR bundle file (`Textures.xpr`) with all textures pre-converted
to Xbox 360's native `D3DFMT_LIN_A8R8G8B8` format in big-endian byte order.

This eliminates per-file disk I/O and runtime image decoding, significantly
reducing texture load times and UI stuttering.

## Building the Tool

XBMC360Tex is a **PC-side build tool** (Win32 x86). It requires:

- **DirectX 9 SDK** (for D3DX image loading via the Surface class)
- **LZO library** (uses the existing)

### Source Files

All files are in `XBMC-360/tools/XBMC360Tex/`:

| File | Purpose |
|------|---------|
| `XBMC360Tex.cpp` | Main tool - image conversion and directory traversal |
| `Bundler360.h/.cpp` | LZO compression and XPR bundle assembly |
| `xbox360.h` | Xbox 360 texture format definitions and byte-swap utilities |

The tool also reuses these files from `xbmc4xbox/tools/XBMCTex/`:

| File | Purpose |
|------|---------|
| `AnimatedGif.h/.cpp` | GIF animation loading |
| `Surface.h/.cpp` | D3D9 surface wrapper for image loading |
| `cmdlineargs.h` | Command line argument parsing |

Copy or add these as existing items to your project.

## Usage

```
XBMC360Tex.exe -input <media_directory> [-output <filename>]
```

### Arguments

| Flag | Description | Default |
|------|-------------|---------|
| `-input <dir>` | Directory containing source images | (required) |
| `-output <file>` | Output bundle filename | `Textures.xpr` |
| `-help` | Show usage | |

### Example

```bash
# Pack the skin's media folder into a bundle
XBMC360Tex.exe -input "skins\Project Mayhem III\media" -output "Textures.xpr"

# Place the output file in the skin's media directory on Xbox 360
# e.g., game:\skins\Project Mayhem III\media\Textures.xpr
```

## Bundle Format (Version 5)

The output XPR file uses version 5 to distinguish from original Xbox bundles:

```
+----------------------------------+
| XPR_FILE_HEADER (12 bytes, BE)   |  Magic: XPR5, TotalSize, HeaderSize
+----------------------------------+
| FileHeader_t[N] (128 bytes each) |  Name[116], Offset, UnpackedSize, PackedSize (BE)
+----------------------------------+
| Padding to 512-byte alignment    |
+----------------------------------+
| LZO-compressed file data blocks  |  Each 512-byte aligned
+----------------------------------+
```

Each decompressed file block contains:

```
+----------------------------------+
| XPR360_FileInfo (12 bytes, BE)   |  nImages, nLoops, TotalSize
+----------------------------------+
| XPR360_TextureInfo[N] (32B each) |  Width, Height, Format, Pitch, etc. (BE)
+----------------------------------+
| Padding to 128-byte alignment    |
+----------------------------------+
| Pixel data (A8R8G8B8, BE)        |  256-byte pitch aligned, big-endian pixels
+----------------------------------+
```

### Key Differences from Original Xbox XPR

| Aspect | Xbox OG (v2) | Xbox 360 (v5) |
|--------|-------------|---------------|
| Byte order | Little-endian | Big-endian |
| Texture header | D3DTexture struct (20B) | XPR360_TextureInfo (32B) |
| Pixel format | Swizzled A8R8G8B8 / P8 | Linear A8R8G8B8 only |
| Palette (P8) | Supported | Not supported |
| Pitch alignment | 64-pixel (swizzled) | 256-byte (linear) |
| Resource binding | Register() | D3DXCreateTexture + memcpy |

## Runtime Integration

The runtime TextureBundle class (`xbmc360/guilib/TextureBundle.cpp`) is
automatically used by the TextureManager when a `Textures.xpr` file is present
in the skin's media directory.

### Files Added to xbmc360

| File | Purpose |
|------|---------|
| `guilib/TextureBundle.h` | Bundle loader class declaration |
| `guilib/TextureBundle.cpp` | Bundle loading, LZO decompression, texture creation |
| `guilib/minilzo.h` | Minimal LZO decompressor header |
| `guilib/minilzo.cpp` | LZO1X safe decompressor implementation |

### Files Modified

| File | Change |
|------|--------|
| `guilib/TextureManager.h` | Added `#include "TextureBundle.h"` and `m_TexBundle[2]` members |
| `guilib/TextureManager.cpp` | Bundle check in `HasTexture()`, bundle loading in `Load()`, cleanup |

### How It Works

1. **Startup**: TextureManager creates two CTextureBundle instances (skin + theme)
2. **HasTexture()**: Checks loaded cache → bundle index → filesystem (in that order)
3. **Load()**: If texture is in a bundle, loads via `CTextureBundle::LoadTexture()`:
   - Seeks to offset in XPR file
   - Reads LZO-compressed block
   - Decompresses to memory
   - Parses XPR360_TextureInfo header
   - Creates D3D9 texture via `D3DXCreateTexture()`
   - Copies pre-converted pixel data directly (no image decode needed)
4. **Fallback**: If no bundle or bundle load fails, falls back to file-based loading

### Performance Benefits

- **No filesystem overhead**: Single file open at startup, offset-based reads thereafter
- **No image decoding**: Textures are pre-converted to GPU-native format
- **LZO decompression**: ~10x faster than PNG/zlib decompression
- **Reduced memory allocation**: Fewer small allocations from individual file loads
- **Graceful fallback**: Missing bundle = original file-based behavior preserved

## Adding to Visual Studio Project

Add these files to your `xbmc360.vcxproj`:

```xml
<!-- In the ClCompile ItemGroup -->
<ClCompile Include="guilib\TextureBundle.cpp" />
<ClCompile Include="guilib\minilzo.cpp" />

<!-- In the ClInclude ItemGroup -->
<ClInclude Include="guilib\TextureBundle.h" />
<ClInclude Include="guilib\minilzo.h" />
```

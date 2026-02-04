/*
  LICENSE
  -------
Copyright 2005-2013 Nullsoft, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer. 

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution. 

  * Neither the name of Nullsoft nor the names of its contributors may be used to 
    endorse or promote products derived from this software without specific prior written permission. 
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "support.h"
#include "utility.h"

D3DXMATRIX matWVP;

void PrepareFor2DDrawing(IDirect3DDevice9 *pDevice)
{
    // New 2D drawing area will have x,y coords in the range <-1,-1> .. <1,1>
    //         +--------+ Y=-1
    //         |        |
    //         | screen |             Z=0: front of scene
    //         |        |             Z=1: back of scene
    //         +--------+ Y=1
    //       X=-1      X=1
    // NOTE: After calling this, be sure to then call (at least):
    //  1. SetVertexShader()
    //  2. SetTexture(), if you need it
    // before rendering primitives!
    // Also, be sure your sprites have a z coordinate of 0.
    pDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
    pDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
    pDevice->SetRenderState( D3DRS_ZFUNC,     D3DCMP_LESSEQUAL );
//    pDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT ); // BRENT
    pDevice->SetRenderState( D3DRS_FILLMODE,  D3DFILL_SOLID );
//    pDevice->SetRenderState( D3DRS_FOGENABLE, FALSE ); // BRENT
    pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
//    pDevice->SetRenderState( D3DRS_CLIPPING, TRUE );  // BRENT
//    pDevice->SetRenderState( D3DRS_LIGHTING, FALSE ); // BRENT
    pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
//    pDevice->SetRenderState( D3DRS_LOCALVIEWER, FALSE ); // BRENT
    
    pDevice->SetTexture(0, NULL);
    pDevice->SetTexture(1, NULL);
    pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);//D3DTEXF_LINEAR);
    pDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_POINT);//D3DTEXF_LINEAR);
//    pDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE); // BRENT
//    pDevice->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);     // BRENT
//    pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE ); // BRENT
//    pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE ); // BRENT
//    pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT ); // BRENT
//    pDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE ); // BRENT

//    pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 ); // BRENT
//    pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE ); // BRENT
//    pDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE ); // BRENT

    pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    
    // set up for 2D drawing:
    {
        D3DXMATRIX Ortho2D;    
        D3DXMATRIX Identity;
        
        D3DXMatrixOrthoLH(&Ortho2D, 2.0f, -2.0f, 0.0f, 1.0f);
        D3DXMatrixIdentity(&Identity);

#ifdef _XBOX
		matWVP = Ortho2D * Identity * Identity;
		pDevice->SetVertexShaderConstantF( 0, (FLOAT*)&matWVP, 4 );
#else
		pDevice->SetTransform(D3DTS_PROJECTION, &Ortho2D);
		pDevice->SetTransform(D3DTS_WORLD, &Identity);
		pDevice->SetTransform(D3DTS_VIEW, &Identity);
#endif
    }
}

//---------------------------------------------------



int GetDX9TexFormatBitsPerPixel(D3DFORMAT fmt)
{
    switch(fmt)
    {
    case D3DFMT_DXT1:   // 64 bits for each 4x4 pixels = 4 bits per pixel.  No Alpha channel.
      return 4; // bytes per pixel
  
    case D3DFMT_DXT2:   // 128 bits for each 4x4 pixels = 8 bits per pixel.  RGB+A.
//    case D3DFMT_DXT3:   // 128 bits for each 4x4 pixels = 8 bits per pixel.  RGB+A.
    case D3DFMT_DXT4:   // 128 bits for each 4x4 pixels = 8 bits per pixel.  RGB+A.
//    case D3DFMT_DXT5:   // 128 bits for each 4x4 pixels = 8 bits per pixel.  RGB+A.
  //  case D3DFMT_R3G3B2: // 8-bit RGB texture format using 3 bits for red, 3 bits for green, and 2 bits for blue. 
    case D3DFMT_A8:   // 8-bit alpha only. 
  //  case D3DFMT_A8P8: // 8-bit color indexed with 8 bits of alpha. 
  //  case D3DFMT_P8:   // 8-bit color indexed. 
    case D3DFMT_L8:   // 8-bit luminance only. 
   // case D3DFMT_A4L4: // 8-bit using 4 bits each for alpha and luminance. 
      return 8;

    case D3DFMT_R5G6B5:   // 16-bit RGB pixel format with 5 bits for red, 6 bits for green, and 5 bits for blue. 
    case D3DFMT_X1R5G5B5: // 16-bit pixel format where 5 bits are reserved for each color. 
    case D3DFMT_A1R5G5B5: // 16-bit pixel format where 5 bits are reserved for each color and 1 bit is reserved for alpha. 
    case D3DFMT_A4R4G4B4: // 16-bit ARGB pixel format with 4 bits for each channel. 
    case D3DFMT_R16F:
//    case D3DFMT_A8R3G3B2: // 16-bit ARGB texture format using 8 bits for alpha, 3 bits each for red and green, and 2 bits for blue. 
    case D3DFMT_X4R4G4B4: // 16-bit RGB pixel format using 4 bits for each color. 
    case D3DFMT_L16:      // 16-bit luminance only. 
    case D3DFMT_A8L8:     // 16-bit using 8 bits each for alpha and luminance. 
//    case D3DFMT_CxV8U8:
    case D3DFMT_V8U8:
    case D3DFMT_L6V5U5:
      return 16;

    case D3DFMT_G16R16F: 
    case D3DFMT_R32F:          // 32-bit float format using 32 bits for the red channel. 
    case D3DFMT_A8R8G8B8:      // 32-bit ARGB pixel format with alpha, using 8 bits per channel. 
    case D3DFMT_X8R8G8B8:      // 32-bit RGB pixel format, where 8 bits are reserved for each color. 
    case D3DFMT_A8B8G8R8:      // 32-bit ARGB pixel format with alpha, using 8 bits per channel. 
    case D3DFMT_X8B8G8R8:      // 32-bit RGB pixel format, where 8 bits are reserved for each color. 
    case D3DFMT_G16R16:        // 32-bit pixel format using 16 bits each for green and red. 
    case D3DFMT_A2R10G10B10:   // 32-bit pixel format using 10 bits each for red, green, and blue, and 2 bits for alpha. 
    case D3DFMT_A2B10G10R10:   // 32-bit pixel format using 10 bits for each color and 2 bits for alpha. 
//    case D3DFMT_R8G8B8:        // 24-bit RGB pixel format with 8 bits per channel. 
    case D3DFMT_X8L8V8U8:
    case D3DFMT_Q8W8V8U8:
    case D3DFMT_V16U16:
      return 32;
  
    case D3DFMT_A16B16G16R16F: 
    case D3DFMT_A16B16G16R16:  // 64-bit pixel format using 16 bits for each component. 
    case D3DFMT_G32R32F:       // 64-bit float format using 32 bits for the red channel and 32 bits for the green channel. 
      return 64;

    case D3DFMT_A32B32G32R32F: 
      return 128;
    }
    
    return 32;
}
#pragma once
#include <xtl.h>

//-------------------------------------------------------------------------------------
// Name: InitD3D()
// Desc: Initializes Direct3D
//-------------------------------------------------------------------------------------
HRESULT InitD3D();

//-------------------------------------------------------------------------------------
// Name: InitScene()
// Desc: Creates the scene.  First we compile our shaders. For the final version
//       of a game, you should store the shaders in binary form; don't call 
//       D3DXCompileShader at runtime. Next, we declare the format of our 
//       vertices, and then create a vertex buffer. The vertex buffer is basically
//       just a chunk of memory that holds vertices. After creating it, we must 
//       Lock()/Unlock() it to fill it. Finally, we set up our world, projection,
//       and view matrices.
//-------------------------------------------------------------------------------------
HRESULT InitScene();

//-------------------------------------------------------------------------------------
// Name: Update()
// Desc: Updates the world for the next frame
//-------------------------------------------------------------------------------------
void Update();

//-------------------------------------------------------------------------------------
// Name: Render()
// Desc: Draws the scene
//-------------------------------------------------------------------------------------
void Render();


HRESULT InitYuvSurface(
	D3DTexture **pFrameY,D3DTexture **pFrameU,D3DTexture **pFrameV,
	int width, int height
);


HRESULT AddFrameToQueue(
	FFMpegData * data
);
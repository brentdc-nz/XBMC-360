#include "GUITextureD3D.h"
#include "GraphicContext.h"

namespace D3DTextureShaders
{
// Vertex shader
const char* g_strVertexShader =
    " float4x4 matWVP : register(c0);              "
    "                                              "
    " struct VS_IN                                 "
    " {                                            "
    "     float4 ObjPos   : POSITION;              "  // Object space position 
    "     float2 TexCoord : TEXCOORD0;             "  // Note the TEXCOORD0 semantic
    " };                                           "
    "                                              "
    " struct VS_OUT                                "
    " {                                            "
    "     float4 ProjPos  : POSITION;              "  // Projected space position 
    "     float2 TexCoord : TEXCOORD0;             "  // Note the TEXCOORD0 semantic
    " };                                           "
    "                                              "
    " VS_OUT main( VS_IN In )                      "
    " {                                            "
    "     VS_OUT Out;                              "
    "     Out.ProjPos = mul( matWVP, In.ObjPos );  "  // Transform vertex into
    "     Out.TexCoord = float2(clamp(In.TexCoord.x, 0.0, 1.0), clamp(In.TexCoord.y, 0.0, 1.0)); "
    "     return Out;                              "
    " }                                            ";

// Pixel shader
const char* g_strPixelShader =
    " struct PS_IN                                  "
    " {                                             "
    "     float2 TexCoord : TEXCOORD;               "
    " };                                            "
    "                                               "
    " float InputAlpha : register(c10);             "
    " sampler2D InputTexture : register(S0);        "
    "                                               "
    " float4 main(float2 uv : TEXCOORD) : COLOR     "
    " {                                             "
    "    float4 color = tex2D( InputTexture, uv.xy);"
    "    color[3] = InputAlpha * color[3];          "
    "    return color;                              "
    " }                                             ";
}

CGUITextureD3D::CGUITextureD3D(float posX, float posY, float width, float height, const CTextureInfo &texture)
: CGUITextureBase(posX, posY, width, height, texture)
{
	m_pVB = NULL;
	m_pVertexShader = NULL;
	m_pVertexDecl = NULL;
	m_pPixelShader = NULL;
}

void CGUITextureD3D::Allocate()
{
	m_pd3dDevice = g_graphicsContext.Get3DDevice();

	if(!m_pd3dDevice)
		return;

	// Compile vertex shader.
	ID3DXBuffer* pVertexShaderCode;
	ID3DXBuffer* pVertexErrorMsg;
   
	D3DXCompileShader( D3DTextureShaders::g_strVertexShader,
                                    ( UINT )strlen( D3DTextureShaders::g_strVertexShader ),
                                    NULL,
                                    NULL,
                                    "main",
                                    "vs_2_0",
                                    0,
                                    &pVertexShaderCode,
                                    &pVertexErrorMsg,
                                    NULL );

    // Create vertex shader.
    m_pd3dDevice->CreateVertexShader( ( DWORD* )pVertexShaderCode->GetBufferPointer(),
                                      &m_pVertexShader );

    // Compile pixel shader.
	ID3DXBuffer* pPixelShaderCode;
    ID3DXBuffer* pPixelErrorMsg;

	D3DXCompileShader( D3DTextureShaders::g_strPixelShader,
                            ( UINT )strlen( D3DTextureShaders::g_strPixelShader ),
                            NULL,
                            NULL,
                            "main",
                            "ps_2_0",
                            0,
                            &pPixelShaderCode,
                            &pPixelErrorMsg,
                            NULL );

    // Create pixel shader.
    m_pd3dDevice->CreatePixelShader( ( DWORD* )pPixelShaderCode->GetBufferPointer(),
                                     &m_pPixelShader );

	// Define the vertex elements and
	// Create a vertex declaration from the element descriptions.
	static const D3DVERTEXELEMENT9 VertexElements[] =
	{
		{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

    m_pd3dDevice->CreateVertexDeclaration( VertexElements, &m_pVertexDecl );

    // Create the vertex buffer. Here we are allocating enough memory
    // (from the default pool) to hold all our 3 custom vertices. 
    m_pd3dDevice->CreateVertexBuffer(4 * sizeof(D3DCUSTOMVERTEX),
                                                  D3DUSAGE_WRITEONLY,
                                                  NULL,
                                                  D3DPOOL_MANAGED,
                                                  &m_pVB,
                                                  NULL);
	if(pVertexShaderCode)
		pVertexShaderCode->Release();

	if(pVertexErrorMsg)
		pVertexErrorMsg->Release();

	if(pPixelShaderCode)
		pPixelShaderCode->Release();

	if(pPixelErrorMsg)
		pPixelErrorMsg->Release();
};

void CGUITextureD3D::Free()
{
	if(m_pVB != NULL)
	{
		g_graphicsContext.TLock();
		m_pd3dDevice->SetStreamSource( NULL, NULL, NULL, NULL );
		g_graphicsContext.TUnlock();

		g_graphicsContext.TLock();
		m_pVB->Release();
		g_graphicsContext.TUnlock();
		g_graphicsContext.TLock();
		m_pVB = NULL;
		g_graphicsContext.TUnlock();
	}

	if(m_pVertexShader != NULL)
	{
		g_graphicsContext.TLock();
		m_pd3dDevice->SetVertexShader( NULL );
		g_graphicsContext.TUnlock();

		g_graphicsContext.TLock();
		m_pVertexShader->Release();
		g_graphicsContext.TUnlock();
		g_graphicsContext.TLock();
		m_pVertexShader = NULL;
		g_graphicsContext.TUnlock();
	}

	if(m_pVertexDecl != NULL)
	{
		g_graphicsContext.TLock();
		m_pd3dDevice->SetVertexDeclaration( NULL );
		g_graphicsContext.TUnlock();

		g_graphicsContext.TLock();
		m_pVertexDecl->Release();
		g_graphicsContext.TUnlock();
		g_graphicsContext.TLock();
		m_pVertexDecl = NULL;
		g_graphicsContext.TUnlock();
	}

	if(m_pPixelShader != NULL)
	{
		g_graphicsContext.TLock();
		m_pd3dDevice->SetPixelShader( NULL );
		g_graphicsContext.TUnlock();

		g_graphicsContext.TLock();
		m_pPixelShader->Release();
		g_graphicsContext.TUnlock();
		g_graphicsContext.TLock();
		m_pPixelShader = NULL;
		g_graphicsContext.TUnlock();
	}
};

void CGUITextureD3D::Begin()
{
	LPDIRECT3DDEVICE9 p3DDevice = g_graphicsContext.Get3DDevice();

	// Set state to render the image

#if 0 //def HAS_XBOX_D3D
	if (!m_texture.m_texCoordsArePixels)
		p3DDevice->SetPalette( 0, m_texture.m_palette);

	if (m_diffuse.m_palette)
		p3DDevice->SetPalette( 1, m_diffuse.m_palette);
#endif

	g_graphicsContext.TLock();
	p3DDevice->SetTexture( 0, m_texture.m_textures[m_currentFrame] );

	p3DDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	p3DDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

	g_graphicsContext.TUnlock();

#if 0 // OG Xbox DX8
	p3DDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
	p3DDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
	p3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	p3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	p3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	p3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	p3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	p3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	p3DDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP );
	p3DDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP );
#endif	
	if (m_diffuse.size())
	{
		g_graphicsContext.TLock();
		p3DDevice->SetTexture( 1, m_diffuse.m_textures[0] );

		p3DDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		p3DDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

		g_graphicsContext.TUnlock();

#if 0 // OG Xbox DX8
		p3DDevice->SetTextureStageState( 1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
		p3DDevice->SetTextureStageState( 1, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
		p3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		p3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
		p3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MODULATE );
		p3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		p3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
		p3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
		p3DDevice->SetTextureStageState( 1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP );
		p3DDevice->SetTextureStageState( 1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP );
#endif
	}

	g_graphicsContext.TLock();
	p3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
	g_graphicsContext.TUnlock();
	g_graphicsContext.TLock();
	p3DDevice->SetRenderState( D3DRS_ALPHAREF, 0 );
	g_graphicsContext.TUnlock();
	g_graphicsContext.TLock();
	p3DDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
	g_graphicsContext.TUnlock();
	g_graphicsContext.TLock();
	p3DDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
	g_graphicsContext.TUnlock();
#if 0 // OG Xbox DX8
	p3DDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );
	p3DDevice->SetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_NONE );
#endif
	g_graphicsContext.TLock();
	p3DDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	g_graphicsContext.TUnlock();
	g_graphicsContext.TLock();
	p3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	g_graphicsContext.TUnlock();
	g_graphicsContext.TLock();
	p3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	g_graphicsContext.TUnlock();
	g_graphicsContext.TLock();
	p3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	g_graphicsContext.TUnlock();
	g_graphicsContext.TLock();
	p3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	g_graphicsContext.TUnlock();
#if 0 // OG Xbox DX8
	p3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE);
#endif
#if 0 //def HAS_XBOX_D3D
	p3DDevice->SetRenderState( D3DRS_YUVENABLE, FALSE);
#endif
#if 0 // OG Xbox DX8
	p3DDevice->SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2 );
#endif
}

void CGUITextureD3D::End()
{
	LPDIRECT3DDEVICE9 p3DDevice = g_graphicsContext.Get3DDevice();

#if 1//def HAS_XBOX_D3D
	if (g_graphicsContext.RectIsAngled(m_vertex.x1, m_vertex.y1, m_vertex.x2, m_vertex.y2))
	{
		p3DDevice->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, FALSE );
#if 0 // OG Xbox DX8
		p3DDevice->SetRenderState( D3DRS_EDGEANTIALIAS, TRUE );
#endif
		p3DDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
		float u = m_frameWidth, v = m_frameHeight;

		if (!m_texture.m_texCoordsArePixels)
		{
			u *= m_texCoordsScaleU;
			v *= m_texCoordsScaleV;
		}

		Render(m_vertex.x1, m_vertex.y1, m_vertex.x2, m_vertex.y2, 0, 0, u, v, u, v);

		p3DDevice->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, TRUE );
#if 0 // OG Xbox DX8
		p3DDevice->SetRenderState( D3DRS_EDGEANTIALIAS, FALSE );
#endif
		p3DDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	}

#if 0 // OG Xbox DX8
	p3DDevice->SetPalette( 0, NULL);
	
	if (m_diffuse.m_palette)
		p3DDevice->SetPalette( 1, NULL);
#endif
#endif // HAS_XBOX_D3D
	
	// Unset the texture and palette or the texture caching crashes because the runtime still has a reference
	g_graphicsContext.TLock();
	g_graphicsContext.Get3DDevice()->SetTexture( 0, NULL );
	g_graphicsContext.TUnlock();

	if (m_diffuse.size())
	{	
		g_graphicsContext.TLock();
		g_graphicsContext.Get3DDevice()->SetTexture( 1, NULL );
		g_graphicsContext.TUnlock();
	}
}

void CGUITextureD3D::Draw(float *x, float *y, float *z, const CRect &texture, const CRect &diffuse, color_t color, int orientation)
{
	struct CUSTOMVERTEX
	{
		FLOAT x, y, z;
		DWORD color;
		FLOAT tu, tv; // Texture coordinates
		FLOAT tu2, tv2;
	};

	// D3D aligns to half pixel boundaries
	for (int i = 0; i < 4; i++)
	{
		x[i] -= 0.5f;
		y[i] -= 0.5f;
	};

	CUSTOMVERTEX verts[4];
	verts[0].x = x[0]; verts[0].y = y[0]; verts[0].z = z[0];
	verts[0].tu = texture.x1;   verts[0].tv = texture.y1;
	verts[0].tu2 = diffuse.x1;  verts[0].tv2 = diffuse.y1;
	verts[0].color = color;

	verts[1].x = x[1]; verts[1].y = y[1]; verts[1].z = z[1];
	if (orientation & 4)
	{
		verts[1].tu = texture.x1;
		verts[1].tv = texture.y2;
	}
	else
	{
		verts[1].tu = texture.x2;
		verts[1].tv = texture.y1;
	}
	if (m_info.orientation & 4)
	{
		verts[1].tu2 = diffuse.x1;
		verts[1].tv2 = diffuse.y2;
	}
	else
	{
		verts[1].tu2 = diffuse.x2;
		verts[1].tv2 = diffuse.y1;
	}

	verts[1].color = color;

	verts[2].x = x[2]; verts[2].y = y[2]; verts[2].z = z[2];
	verts[2].tu = texture.x2;   verts[2].tv = texture.y2;
	verts[2].tu2 = diffuse.x2;  verts[2].tv2 = diffuse.y2;
	verts[2].color = color;

	verts[3].x = x[3]; verts[3].y = y[3]; verts[3].z = z[3];

	if (orientation & 4)
	{
		verts[3].tu = texture.x2;
		verts[3].tv = texture.y1;
	}
	else
	{
		verts[3].tu = texture.x1;
		verts[3].tv = texture.y2;
	}

	if (m_info.orientation & 4)
	{
		verts[3].tu2 = diffuse.x2;
		verts[3].tv2 = diffuse.y1;
	}
	else
	{
		verts[3].tu2 = diffuse.x1;
		verts[3].tv2 = diffuse.y2;
	}

	verts[3].color = color;

	D3DCUSTOMVERTEX Vertices[] =
    {
		{x[1], y[1],  z[1], 1, 0, },
		{x[0], y[0],  z[0], 0, 0, },
		{x[2], y[2],  z[2], 1, 1, },
		{x[3], y[3],  z[3], 0, 1  },
    };

	D3DCUSTOMVERTEX* pVertices;
	g_graphicsContext.TLock();
	m_pVB->Lock(0, 0, (void** )&pVertices, 0);
	g_graphicsContext.TUnlock();
	g_graphicsContext.TLock();
	memcpy(pVertices, Vertices, 4 * sizeof(D3DCUSTOMVERTEX));
	m_pVB->Unlock();
	g_graphicsContext.TUnlock();

	// Build the world-view-projection matrix and pass it into the vertex shader
	g_graphicsContext.TLock();
	m_pd3dDevice->SetVertexShaderConstantF(0, ( FLOAT* )&g_graphicsContext.GetFinalMatrix(), 4);
	g_graphicsContext.TUnlock();

	g_graphicsContext.TLock();
	m_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
	g_graphicsContext.TUnlock();
	g_graphicsContext.TLock();
	m_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
	g_graphicsContext.TUnlock();
	g_graphicsContext.TLock();
	m_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	g_graphicsContext.TUnlock();

#pragma warning(push)
#pragma warning (disable:4244) // Not an issue here

	float fInputAlpha[] = { (color >> 24 & 0xFF) / 255.0 };

	g_graphicsContext.TLock();
	m_pd3dDevice->SetPixelShaderConstantF(10, fInputAlpha, 4);
	g_graphicsContext.TUnlock();

#pragma warning(pop)

	// We are passing the vertices down a "stream", so first we need
    // to specify the source of that stream, which is our vertex buffer. 
    // Then we need to let D3D know what vertex and pixel shaders to use.
	g_graphicsContext.TLock();
    m_pd3dDevice->SetVertexDeclaration(m_pVertexDecl);
	g_graphicsContext.TUnlock();
	g_graphicsContext.TLock();
	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(D3DCUSTOMVERTEX));
	g_graphicsContext.TUnlock();
	g_graphicsContext.TLock();
	m_pd3dDevice->SetVertexShader(m_pVertexShader);
	g_graphicsContext.TUnlock();
	g_graphicsContext.TLock();
	m_pd3dDevice->SetPixelShader(m_pPixelShader);
	g_graphicsContext.TUnlock();

	g_graphicsContext.TLock();
	m_pd3dDevice->SetTexture(0, m_texture.m_textures[m_currentFrame]);
	g_graphicsContext.TUnlock();

    // Draw the vertices in the vertex buffer
	g_graphicsContext.TLock();
	m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	g_graphicsContext.TUnlock();
	g_graphicsContext.TLock();
	m_pd3dDevice->SetTexture(0,  NULL);
	g_graphicsContext.TUnlock();

	g_graphicsContext.TLock();
	m_pd3dDevice->SetStreamSource(NULL, NULL, NULL, NULL);
	g_graphicsContext.TUnlock();
}

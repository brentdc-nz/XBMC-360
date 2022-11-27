#ifndef GUILIB_GUITEXTURE_H
#define GUILIB_GUITEXTURE_H

#include "TextureManager.h"
#include "Geometry.h"

typedef uint32_t color_t;

struct FRECT
{
	float left;
	float top;
	float right;
	float bottom;
};

// Image alignment for <aspect>keep</aspect>, <aspect>scale</aspect> or <aspect>center</aspect>
#define ASPECT_ALIGN_CENTER  0
#define ASPECT_ALIGN_LEFT    1
#define ASPECT_ALIGN_RIGHT   2
#define ASPECT_ALIGNY_CENTER 0
#define ASPECT_ALIGNY_TOP    4
#define ASPECT_ALIGNY_BOTTOM 8
#define ASPECT_ALIGN_MASK    3
#define ASPECT_ALIGNY_MASK  ~3

class CAspectRatio
{
public:
	enum ASPECT_RATIO { AR_STRETCH = 0, AR_SCALE, AR_KEEP, AR_CENTER };
	
	CAspectRatio(ASPECT_RATIO aspect = AR_STRETCH)
	{
		ratio = aspect;
		align = ASPECT_ALIGN_CENTER | ASPECT_ALIGNY_CENTER;
		scaleDiffuse = true;
	};

	bool operator!=(const CAspectRatio &right) const
	{
		if (ratio != right.ratio) return true;
		if (align != right.align) return true;
		if (scaleDiffuse != right.scaleDiffuse) return true;
		return false;
	};

	ASPECT_RATIO ratio;
	uint32_t     align;
	bool         scaleDiffuse;
};

class CTextureInfo
{
public:
	CTextureInfo()
	{
		memset(&border, 0, sizeof(FRECT));
		orientation = 0;
		useLarge = false;
	};

	CTextureInfo(const CStdString &file)
	{
		memset(&border, 0, sizeof(FRECT));
		orientation = 0;
		useLarge = false;
		filename = file;
	}

	void operator=(const CTextureInfo &right)
	{
		memcpy(&border, &right.border, sizeof(FRECT));
		orientation = right.orientation;
		diffuse = right.diffuse;
		filename = right.filename;
		useLarge = right.useLarge;
	};

	bool       useLarge;
	FRECT      border;      // Scaled  - Unneeded if we get rid of scale on load
	int        orientation; // Orientation of the texture (0 - 7 == EXIForientation - 1)
	CStdString diffuse;     // Diffuse overlay texture
	CStdString filename;    // Main texture file
};

class CGUITextureBase
{
public:
	CGUITextureBase(float posX, float posY, float width, float height, const CTextureInfo& texture);
	CGUITextureBase(const CGUITextureBase &left);
	virtual ~CGUITextureBase(void);

	void Render();

	void DynamicResourceAlloc(bool bOnOff);
	void PreAllocResources();
	void AllocResources();
	void FreeResources(bool immediately = false);

	void SetVisible(bool visible);
	void SetAlpha(unsigned char alpha);
	void SetDiffuseColor(color_t color);
	void SetPosition(float x, float y);
	void SetWidth(float width);
	void SetHeight(float height);
	void SetFileName(const CStdString &filename);
	void SetAspectRatio(const CAspectRatio &aspect);

	const CStdString& GetFileName() const { return m_info.filename; };
	float GetTextureWidth() const { return m_frameWidth; };
	float GetTextureHeight() const { return m_frameHeight; };
	float GetWidth() const { return m_width; };
	float GetHeight() const { return m_height; };
	float GetXPosition() const { return m_posX; };
	float GetYPosition() const { return m_posY; };
	int GetOrientation() const;
	const CRect &GetRenderRect() const { return m_vertex; };
	bool IsLazyLoaded() const { return m_info.useLarge; };

	bool HitTest(const CPoint &point) const { return CRect(m_posX, m_posY, m_posX + m_width, m_posY + m_height).PtInRect(point); };
	bool IsAllocated() const { return m_isAllocated != NO; };
	bool FailedToAlloc() const { return m_isAllocated == NORMAL_FAILED || m_isAllocated == LARGE_FAILED; };
	bool ReadyToRender() const;

protected:
	void CalculateSize();
	void LoadDiffuseImage();
	void AllocateOnDemand();
	void UpdateAnimFrame();
	void Render(float left, float top, float bottom, float right, float u1, float v1, float u2, float v2, float u3, float v3);
	void OrientateTexture(CRect &rect, float width, float height, int orientation);

	// Functions that our implementation classes handle
	virtual void Allocate() {}; // Called after our textures have been allocated
	virtual void Free() {}; // Called after our textures have been freed
	virtual void Begin() {};
	virtual void Draw(float *x, float *y, float *z, const CRect &texture, const CRect &diffuse, color_t color, int orientation)=0;
	virtual void End() {};

	bool m_visible;
	color_t m_diffuseColor;

	float m_posX; // Size of the frame
	float m_posY;
	float m_width;
	float m_height;

	CRect m_vertex; // Vertex coords to render
	bool m_invalid; // If true, we need to recalculate

	unsigned char m_alpha;

	float m_frameWidth, m_frameHeight; // Size in pixels of the actual frame within the texture
	float m_texCoordsScaleU, m_texCoordsScaleV; // Scale factor for pixel->texture coordinates

	// Animations
	int m_currentLoop;
	unsigned int m_currentFrame;
	DWORD m_frameCounter;

	float m_diffuseU, m_diffuseV; // Size of the diffuse frame (in tex coords)
	float m_diffuseScaleU, m_diffuseScaleV; // Scale factor of the diffuse frame (from texture coords to diffuse tex coords)
	CPoint m_diffuseOffset; // Offset into the diffuse frame (it's not always the origin)

	bool m_allocateDynamically;
	enum ALLOCATE_TYPE { NO = 0, NORMAL, LARGE, NORMAL_FAILED, LARGE_FAILED };
	ALLOCATE_TYPE m_isAllocated;

	CTextureInfo m_info;
	CAspectRatio m_aspect;

	int m_largeOrientation; // Orientation for large textures

	CTexture m_diffuse;
	CTexture m_texture;
};

#include "GUITextureD3D.h"

#endif //GUILIB_GUITEXTURE_H

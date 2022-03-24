#ifndef GUILIB_GUITEXTURE_H
#define GUILIB_GUITEXTURE_H

#include "..\utils\StdString.h"

struct FRECT
{
	float left;
	float top;
	float right;
	float bottom;
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
	FRECT      border;      // scaled  - unneeded if we get rid of scale on load
	int        orientation; // orientation of the texture (0 - 7 == EXIForientation - 1)
	CStdString diffuse;     // diffuse overlay texture
	CStdString filename;    // main texture file
};

class CGUITextureBase
{
public:
	float GetTextureWidth() { return m_width; };
	float GetTextureHeight() { return m_height; };
	float GetXPosition() const { return m_posX; };
	float GetYPosition() const { return m_posY; };

protected:
	bool m_bVisible;

	float m_posX;
	float m_posY;
	float m_width;
	float m_height;
};

#endif //GUILIB_GUITEXTURE_H
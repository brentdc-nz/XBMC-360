// Stdafx.h.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <xtl.h>
#include <xonline.h>
#include <XGraphics.h>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <memory>
#include "StringUtils.h"

// TODO: reference additional headers your program requires here

#define SAFE_DELETE(p)       { delete (p);     (p)=NULL; }
#define SAFE_DELETE_ARRAY(p) { delete[] (p);   (p)=NULL; }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
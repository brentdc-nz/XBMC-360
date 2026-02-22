#pragma once

#include "ICodec.h"
#include "FileSystem\File.h"

class CachingCodec : public ICodec
{
public:
  virtual ~CachingCodec() {}
  virtual int GetCacheLevel() const { return -1; }

protected:
  XFILE::CFile m_file;
};
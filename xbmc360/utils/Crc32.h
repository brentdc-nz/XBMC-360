#pragma once

#include "StdString.h"
#include "stdint.h"

class Crc32
{
public:
	Crc32();
	void Reset();
	void Compute(const char* buffer, size_t count);
	void Compute(const CStdString& strValue);
	void ComputeFromLowerCase(const CStdString& strValue);

	operator uint32_t () const
	{
		return m_crc;
	}

private:
	uint32_t m_crc;
};

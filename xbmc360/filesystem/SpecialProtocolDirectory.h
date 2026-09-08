#pragma once

#include "IDirectory.h"

namespace XFILE
{
	class CSpecialProtocolDirectory : public IDirectory
	{
	public:
		CSpecialProtocolDirectory(void);
		virtual ~CSpecialProtocolDirectory(void);
		
		virtual bool GetDirectory(const CStdString& strPath, CFileItemList &items);
		virtual bool Create(const char* strPath);
		virtual bool Exists(const char* strPath);
		virtual bool Remove(const char* strPath);
	};
}

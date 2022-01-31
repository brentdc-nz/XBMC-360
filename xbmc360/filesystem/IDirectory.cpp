#include "IDirectory.h"
#include "..\utils\Util.h"

using namespace DIRECTORY;

IDirectory::IDirectory(void)
{
	m_strFileMask = "";
	m_allowPrompting = false;
	m_cacheDirectory = false;
}

IDirectory::~IDirectory(void)
{
}

// Test a file for an extension specified with SetMask().
// strFile File to test
// Return true, if file is allowed
bool IDirectory::IsAllowed(const CStdString& strFile)
{
	CStdString strExtension;

	if(!m_strFileMask.size()) return true;
	if(!strFile.size()) return true;

	CUtil::GetExtension(strFile, strExtension);

	if(!strExtension.size()) return false;
		strExtension.ToLower();

	bool bOkay = false;
	int i = -1;
	
	while(!bOkay)
	{
		i = m_strFileMask.Find(strExtension,i+1);

		if(i >= 0)
		{
			if(i+strExtension.size() == m_strFileMask.size())
				bOkay = true;
			else
			{
				char c = m_strFileMask[i+strExtension.size()];
				if(c == '|')
				bOkay = true;
			else
				bOkay = false;
			}
		}    
		else
			break;
	}
	if(i >= 0 && bOkay)
	{
		return true;
	}
	return false;
}

// Set a mask of extensions for the files in the directory
// Param strMask Mask of file extensions that are allowed
// The mask has to look like the following:
// .m4a|.flac|.aac|
// So only *.m4a, *.flac, *.aac files will be retrieved with GetDirectory()
void IDirectory::SetMask(const CStdString& strMask)
{
	m_strFileMask = strMask;
	m_strFileMask.ToLower();
}
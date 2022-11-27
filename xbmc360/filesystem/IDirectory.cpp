#include "IDirectory.h"
#include "utils\Util.h"
#include "utils\URIUtils.h"

using namespace XFILE;

IDirectory::IDirectory(void)
{
	m_strFileMask = "";
	m_allowPrompting = false;
	m_cacheDirectory = DIR_CACHE_NEVER;
	m_useFileDirectories = false;
	m_extFileInfo = true;
}

IDirectory::~IDirectory(void)
{
}

// Test a file for an extension specified with SetMask().
// param strFile File to test
// return Returns \e true, if file is allowed
bool IDirectory::IsAllowed(const CStdString& strFile) const
{
	CStdString strExtension;
	if ( !m_strFileMask.size() ) return true;
	if ( !strFile.size() ) return true;

	URIUtils::GetExtension(strFile, strExtension);

	if (!strExtension.size()) return false;

	strExtension.ToLower();
	strExtension += '|'; // Ensures that we have a | at the end of it

	if ((size_t)m_strFileMask.Find(strExtension) != -1)
	{
		// It's allowed, but we should also ignore all non DVD related ifo files
		if (strExtension.Equals(".ifo|"))
		{
			CStdString fileName = URIUtils::GetFileName(strFile);
			if (fileName.Equals("video_ts.ifo")) return true;
			if (fileName.length() == 12 && fileName.Left(4).Equals("vts_") && fileName.Right(6).Equals("_0.ifo")) return true;

			return false;
		}
		return true;
	}
	return false;
}

// Set a mask of extensions for the files in the directory.
// param strMask Mask of file extensions that are allowed.

// The mask has to look like the following: \n
// \verbatim
// .m4a|.flac|.aac|
// \endverbatim
// So only *.m4a, *.flac, *.aac files will be retrieved with GetDirectory()
void IDirectory::SetMask(const CStdString& strMask)
{
	m_strFileMask = strMask;

	// Ensure it's completed with a | so that filtering is easy
	m_strFileMask.ToLower();
	
	if (m_strFileMask.size() && m_strFileMask[m_strFileMask.size() - 1] != '|')
		m_strFileMask += '|';
}

// Set whether the directory handlers can prompt the user.
// param allowPrompting Set true to allow prompting to occur (default is false)
 
// Directory handlers should only prompt the user as a direct result of the
// users actions
void IDirectory::SetAllowPrompting(bool allowPrompting)
{
	m_allowPrompting = allowPrompting;
}

// Set whether the directory should be cached by our directory cache.
// param cacheDirectory Set DIR_CACHE_ONCE or DIR_CACHE_ALWAYS to enable caching (default is DIR_CACHE_NEVER)
void IDirectory::SetCacheDirectory(DIR_CACHE_TYPE cacheDirectory)
{
	m_cacheDirectory = cacheDirectory;
}

// Set whether the directory should allow file directories.
// param useFileDirectories Set true to enable file directories (default is true)
void IDirectory::SetUseFileDirectories(bool useFileDirectories)
{
	m_useFileDirectories = useFileDirectories;
}

// Set whether the GetDirectory call will retrieve extended file information (stat calls for example)
// param extFileInfo Set true to enable extended file info (default is true)
void IDirectory::SetExtFileInfo(bool extFileInfo)
{
	m_extFileInfo = extFileInfo;
}
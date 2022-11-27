#ifndef CUTIL_H
#define CUTIL_H

#include "StdString.h"
#include "stdafx.h"
#include "MediaSource.h"

class CUtil
{
public:

	static void SplitExecFunction(const CStdString &execString, CStdString &function, std::vector<CStdString> &parameters);
	static void SplitParams(const CStdString &paramString, std::vector<CStdString> &parameters);
	static const CStdString GetFileName(const CStdString& strFileNameAndPath);
	static CStdString GetTitleFromPath(const CStdString& strFileNameAndPath, bool bIsFolder = false);

	static bool GetParentPath(const CStdString& strPath, CStdString& strParent);
	static bool HasSlashAtEnd(const CStdString& strFile);
	static void AddSlashAtEnd(CStdString& strFolder);
	static const CStdString GetExtension(const CStdString& strFileName);
	static void GetExtension(const CStdString& strFile, CStdString& strExtension);
	static void RemoveSlashAtEnd(CStdString& strFolder);
	static bool IsLocalDrive(const CStdString& strPath, bool bFullPath = false);
	static bool FileExists(CStdString strFullPath) { return (GetFileAttributes(strFullPath.c_str()) != 0xFFFFFFFF); } // TODO : Remove once filesystem is more sorted out
	static int GetMatchingShare(const CStdString& strPath, VECSOURCES& vecShares, bool& bIsBookmarkName);
	static void ForceForwardSlashes(CStdString& strPath);
	static void Stat64ToStat(struct _stat *result, struct __stat64 *stat);
	static void URLEncode(CStdString& strURLData);
	static void AddFileToFolder(const CStdString& strFolder, const CStdString& strFile, CStdString& strResult);
	static bool IsStack(const CStdString& strFile);
	static bool IsPicture(const CStdString& strFile);
	static void UrlDecode(CStdString& strURLData);
	static float CurrentCpuUsage();
	static __int64 ToInt64(DWORD dwHigh, DWORD dwLow);
	static bool ExcludeFileOrFolder(const CStdString& strFileOrFolder, const CStdStringArray& regexps);
	static int GetMatchingSource(const CStdString& strPath, VECSOURCES& VECSOURCES, bool& bIsSourceName);
};

#endif //CUTIL_H
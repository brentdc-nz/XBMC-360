#ifndef CFILEBROWSER_H
#define CFILEBROWSER_H

#include "utils\StdString.h"
#include "File.h"
#include <vector>

class CFileBrowser : public XFILE::CFile
{
public:
	CFileBrowser();
	~CFileBrowser();

	CStdString GetCurrentPath();
	bool IsAtRoot();
	void UpToRoot();
	void UpDirectory();
	CStdString GetWriteFilePath(CStdString strFsile);
	FILE* CFileBrowser::OpenFile(CStdString strFile);
	virtual std::vector<CStdString> GetFolderList();
	virtual std::vector<CStdString> GetFileList();
	CStdString GetFileFTPLongDescription(CStdString strFile);
	CStdString GetFolderFTPLongDescription(CStdString strFolder);
	void CD(CStdString strFolder);

private:
	std::vector<CStdString> m_vecCurrentFolder;
};

#endif //CFILEBROWSER_H
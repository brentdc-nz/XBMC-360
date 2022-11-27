#ifndef CVIRTUALDIRECTORY_H
#define CVIRTUALDIRECTORY_H
#include "filesystem\IDirectory.h"
#include "Settings.h"

namespace XFILE
{
// Get access to shares and it's directories
class CVirtualDirectory : public IDirectory
{
public:
	CVirtualDirectory(void);
	virtual ~CVirtualDirectory(void);
	virtual bool GetDirectory(const CStdString& strPath, CFileItemList &items);
	virtual bool GetDirectory(const CStdString& strPath, CFileItemList &items, bool bUseFileDirectories); 
	void SetSources(const VECSOURCES& vecSources);

	inline unsigned int GetNumberOfSources() 
	{
		return m_vecSources.size();
	}

	bool IsSource(const CStdString& strPath, VECSOURCES *sources = NULL, CStdString *name = NULL) const;
	bool IsInSource(const CStdString& strPath) const;

	inline const CMediaSource& operator [](const int index) const
	{
		return m_vecSources[index];
	}

	inline CMediaSource& operator[](const int index)
	{
		return m_vecSources[index];
	}

	void GetSources(VECSOURCES &sources) const;

	void AllowNonLocalSources(bool allow) { m_allowNonLocalSources = allow; };

protected:
	void CacheThumbs(CFileItemList &items);

	VECSOURCES m_vecSources;
	bool       m_allowNonLocalSources;
};
}

#endif //CVIRTUALDIRECTORY_H
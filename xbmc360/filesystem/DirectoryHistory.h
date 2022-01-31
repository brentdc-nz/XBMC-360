#ifndef H_CDIRECTORYHISTORY
#define H_CDIRECTORYHISTORY

#include "..\utils\StdString.h"

class CDirectoryHistory
{
public:
	class CHistoryItem
	{
		public:
		CHistoryItem(){};
		virtual ~CHistoryItem(){};

		CStdString m_strItem;
		CStdString m_strDirectory;
	};
	CDirectoryHistory();
	virtual ~CDirectoryHistory();

	CStdString GetParentPath();
	CStdString RemoveParentPath();
	void ClearPathHistory();
	void AddPath(const CStdString& strPath);
	void AddPathFront(const CStdString& strPath);
	void SetSelectedItem(const CStdString& strSelectedItem, const CStdString& strDirectory);
	const CStdString& GetSelectedItem(const CStdString& strDirectory) const;
	void DumpPathHistory();

private:
	std::vector<CHistoryItem> m_vecHistory;
	std::vector<CStdString> m_vecPathHistory; // History of traversed directories
	CStdString m_strNull;
};

#endif //H_CDIRECTORYHISTORY
#pragma once
/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "utils/StdString.h"
#include "sqlitedataset.h"

#include <memory>

class CDatabase
{
public:
	CDatabase(void);
	virtual ~CDatabase(void);
	bool Open();
	bool IsOpen();
	void Close();
	bool Compress(bool bForce=true);
	void Interupt();

	void BeginTransaction();
	virtual bool CommitTransaction();
	void RollbackTransaction();
	bool InTransaction();

	static CStdString FormatSQL(CStdString strStmt, ...);
	CStdString PrepareSQL(CStdString strStmt, ...) const;

	CStdString GetSingleValue(const CStdString &strTable, const CStdString &strColumn, const CStdString &strWhereClause = CStdString(), const CStdString &strOrderBy = CStdString());

	std::string GetSingleValue(const std::string &query, std::auto_ptr<dbiplus::Dataset> &ds);

	bool DeleteValues(const CStdString &strTable, const CStdString &strWhereClause = CStdString());

	bool ExecuteQuery(const CStdString &strQuery);

	bool ResultQuery(const CStdString &strQuery);

	bool QueueInsertQuery(const CStdString &strQuery);

	bool CommitInsertQueries();

protected:
	void Split(const CStdString& strFileNameAndPath, CStdString& strPath, CStdString& strFileName);
	DWORD ComputeCRC(const CStdString &text);

	virtual bool CreateTables();
	virtual bool UpdateOldVersion(int version) { return true; };

	virtual int GetMinVersion() const=0;

	bool m_bOpen;
	CStdString m_strDatabaseFile;
	std::auto_ptr<dbiplus::Database> m_pDB;
	std::auto_ptr<dbiplus::Dataset> m_pDS;
	std::auto_ptr<dbiplus::Dataset> m_pDS2;

private:
	bool UpdateVersionNumber();

	bool m_bMultiWrite;

	int m_iRefCount;
};

#include "DVDInputStream.h"
#include "..\..\..\utils\stdafx.h"

CDVDInputStream::CDVDInputStream()
{
	m_strFileName = NULL;
}

CDVDInputStream::~CDVDInputStream()
{
}

bool CDVDInputStream::Open(const char* strFile)
{
	if (m_strFileName)
		delete m_strFileName;
	
	m_strFileName = strdup(strFile);
	
	return true;
}

void CDVDInputStream::Close()
{
	if (m_strFileName)
		delete m_strFileName;
	
	m_strFileName = NULL;
}

const char* CDVDInputStream::GetFileName()
{
	return m_strFileName;
}
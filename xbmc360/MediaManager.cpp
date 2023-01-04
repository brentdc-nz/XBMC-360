#include "MediaManager.h"
#include "utils\Log.h"
#include "guilib\LocalizeStrings.h"
#include "Application.h"

using namespace std;

const char MEDIA_SOURCES_XML[] = { "D:\\system\\mediasources.xml" };

class CMediaManager g_mediaManager;

CMediaManager::CMediaManager()
{
}

bool CMediaManager::LoadSources()
{
	// Clear our location list
	m_locations.clear();

	// Load xml file...
	TiXmlDocument xmlDoc;
	if ( !xmlDoc.LoadFile( MEDIA_SOURCES_XML ) )
		return false;

	TiXmlElement* pRootElement = xmlDoc.RootElement();
	if ( !pRootElement || strcmpi(pRootElement->Value(), "mediasources") != 0)
	{
		CLog::Log(LOGERROR, "Error loading %s, Line %d (%s)", MEDIA_SOURCES_XML, xmlDoc.ErrorRow(), xmlDoc.ErrorDesc());
		return false;
	}

	// Load the <network> block
	TiXmlNode *pNetwork = pRootElement->FirstChild("network");
	if (pNetwork)
	{
		TiXmlElement *pLocation = pNetwork->FirstChildElement("location");
		while (pLocation)
		{
			CNetworkLocation location;
			pLocation->Attribute("id", &location.id);
			
			if (pLocation->FirstChild())
			{
				location.path = pLocation->FirstChild()->Value();
				m_locations.push_back(location);
			}
			pLocation = pLocation->NextSiblingElement("location");
		}
	}
	return true;
}

bool CMediaManager::SaveSources()
{
	TiXmlDocument xmlDoc;
	TiXmlElement xmlRootElement("mediasources");
	TiXmlNode *pRoot = xmlDoc.InsertEndChild(xmlRootElement);
	
	if (!pRoot) return false;

	TiXmlElement networkNode("network");
	TiXmlNode *pNetworkNode = pRoot->InsertEndChild(networkNode);

	if (pNetworkNode)
	{
		for (vector<CNetworkLocation>::iterator it = m_locations.begin(); it != m_locations.end(); it++)
		{
			TiXmlElement locationNode("location");
			locationNode.SetAttribute("id", (*it).id);
			TiXmlText value((*it).path);
			locationNode.InsertEndChild(value);
			pNetworkNode->InsertEndChild(locationNode);
		}
	}
	return xmlDoc.SaveFile(MEDIA_SOURCES_XML);
}

void CMediaManager::GetLocalDrives(VECSOURCES &localDrives, bool includeQ)
{
	// Get local mounted drives
	std::vector<CDrive* const> vecMountedDrives;
	g_application.getDriveManager().getMountedDrives(&vecMountedDrives);

	for(unsigned int i = 0; i < vecMountedDrives.size(); i++)
	{
		CMediaSource share;
		share.strPath = vecMountedDrives[i]->getRootPath();
		share.strName = "Drive ";//g_localizeStrings.Get(21438); // TODO
		share.strName += vecMountedDrives[i]->GetCleanDriveName();

		if(vecMountedDrives[i]->GetDriveType() == DEVICE_CDROM0)
			int j = 0;
		else
			share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;

		localDrives.push_back(share);
	}
}

void CMediaManager::GetNetworkLocations(VECSOURCES &locations)
{
	// Load our xml file
	LoadSources();

	for (unsigned int i = 0; i < m_locations.size(); i++)
	{
		CMediaSource share;
		share.strPath = m_locations[i].path;
		CURL url(share.strPath);
		share.strName = url.GetWithoutUserDetails();
		locations.push_back(share);
	}
}

bool CMediaManager::AddNetworkLocation(const CStdString &path)
{
	CNetworkLocation location;
	location.path = path;
	location.id = (int)m_locations.size();
	m_locations.push_back(location);
	return SaveSources();
}

bool CMediaManager::HasLocation(const CStdString& path) const
{
	for (unsigned int i=0;i<m_locations.size();++i)
	{
		if (m_locations[i].path == path)
			return true;
	}

	return false;
}

bool CMediaManager::RemoveLocation(const CStdString& path)
{
	for (unsigned int i=0;i<m_locations.size();++i)
	{
		if (m_locations[i].path == path)
		{
			// Prompt for sources, remove, cancel 
			m_locations.erase(m_locations.begin()+i);
			return SaveSources();
		}
	}

	return false;
}

bool CMediaManager::SetLocationPath(const CStdString& oldPath, const CStdString& newPath)
{
	for (unsigned int i=0;i<m_locations.size();++i)
	{
		if (m_locations[i].path == oldPath)
		{
			m_locations[i].path = newPath;
			return SaveSources();
		}
	}
	return false;
}
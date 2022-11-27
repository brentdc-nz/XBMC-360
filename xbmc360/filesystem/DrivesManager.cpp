#include "DrivesManager.h"
#include "utils\Log.h"
#include "utils\StringUtils.h"
#include "xbox\XBKernalExports.h"

CDrivesManager::CDrivesManager()
{
}

CDrivesManager::~CDrivesManager()
{
}

void CDrivesManager::Init()
{
	m_vecDrives.push_back(new CDrive("Flash:", "\\Device\\Flash", DEVICE_NAND_FLASH));
	m_vecDrives.push_back(new CDrive("Memunit0:", "\\Device\\Mu0", DEVICE_MEMORY_UNIT0));
	m_vecDrives.push_back(new CDrive("Memunit1:", "\\Device\\Mu1", DEVICE_MEMORY_UNIT1));
	m_vecDrives.push_back(new CDrive("OnBoardMU:", "\\Device\\BuiltInMuSfc", DEVICE_MEMORY_ONBOARD));
	m_vecDrives.push_back(new CDrive("Dvd:", "\\Device\\Cdrom0", DEVICE_CDROM0));
	m_vecDrives.push_back(new CDrive("Hdd1:", "\\Device\\Harddisk0\\Partition1", DEVICE_HARDISK0_PART1));
	m_vecDrives.push_back(new CDrive("Hdd0:", "\\Device\\Harddisk0\\Partition0", DEVICE_HARDISK0_PART0));
	m_vecDrives.push_back(new CDrive("HddX:", "\\Device\\Harddisk0\\SystemPartition", DEVICE_HARDISK0_SYSPART));
	m_vecDrives.push_back(new CDrive("SysExt:", "\\Device\\Harddisk0\\SystemExtPartition", DEVICE_HARDISK0_SYSEXT));
	m_vecDrives.push_back(new CDrive("Usb0:", "\\Device\\Mass0", DEVICE_USB0));
	m_vecDrives.push_back(new CDrive("Usb1:", "\\Device\\Mass1", DEVICE_USB1));
	m_vecDrives.push_back(new CDrive("Usb2:", "\\Device\\Mass2", DEVICE_USB2));
	m_vecDrives.push_back(new CDrive("HdDvdPlayer:", "\\Device\\HdDvdPlayer", DEVICE_HDDVD_PLAYER));
	m_vecDrives.push_back(new CDrive("HdDvdStorage:", "\\Device\\HdDvdStorage", DEVICE_HDDVD_STORGE));
	m_vecDrives.push_back(new CDrive("Transfercable:", "\\Device\\Transfercable", DEVICE_TRANSFER_CABLE)); // Only with 2.0.9199 Kernel
	m_vecDrives.push_back(new CDrive("TransfercableXbox1:", "\\Device\\Transfercable\\Compatibility\\Xbox1", DEVICE_TRANSFER_CABLE_SYSPART)); // Only with 2.0.9199 Kernel  
	m_vecDrives.push_back(new CDrive("USBMU0:", "\\Device\\Mass0PartitionFile\\Storage", DEVICE_USBMEMORY_UNIT0));
	m_vecDrives.push_back(new CDrive("USBMU1:", "\\Device\\Mass1PartitionFile\\Storage", DEVICE_USBMEMORY_UNIT1));
	m_vecDrives.push_back(new CDrive("USBMU2:", "\\Device\\Mass2PartitionFile\\Storage", DEVICE_USBMEMORY_UNIT2));
	m_vecDrives.push_back(new CDrive("USBMUCache0:", "\\Device\\Mass0PartitionFile\\StorageSystem", DEVICE_USBMEMORY_Cache0));
	m_vecDrives.push_back(new CDrive("USBMUCache1:", "\\Device\\Mass1PartitionFile\\StorageSystem", DEVICE_USBMEMORY_Cache1));
	m_vecDrives.push_back(new CDrive("USBMUCache2:", "\\Device\\Mass2PartitionFile\\StorageSystem", DEVICE_USBMEMORY_Cache2));

	// Set variable to represent current xexpath
	CStdString LoadedImageName(GetLoadedImageName());
	m_strExecutableFilePath = getExecutablePathFromSystemPath(LoadedImageName);
}

bool CDrivesManager::MountAll()
{
	for(unsigned int x = 0; x < m_vecDrives.size(); x++)
	{
		CDrive* pDrive = m_vecDrives[x];
		if(pDrive)
			pDrive->Mount();
	}

	CLog::Log(LOGNOTICE, "Finished mounting the drives..");

	return true; //TODO: Check for failures and return;
}

void CDrivesManager::Unmount()
{
	for(unsigned int x = 0; x < m_vecDrives.size(); x++)
	{
		CDrive* pDrive = m_vecDrives[x];
		if(pDrive)
		{
			pDrive->Unmount();
			delete pDrive;
		}
	}

	CLog::Log(LOGNOTICE, "Unmounting the drives..");
}

void CDrivesManager::getMountedDrives(std::vector<CDrive* const> *vecRetVal)
{
	bool bShowAdvancedDrives = true;//SETTINGS::getInstance().getFMAdvancedMode() == 0 ? false : true; // TODO

	for(unsigned int x = 0; x < m_vecDrives.size(); x++)
	{
		if(m_vecDrives[x]->IsMounted())
		{
			CDrive *pDrvive = m_vecDrives[x];

			if(pDrvive->GetDriveType() == DEVICE_NAND_FLASH && !bShowAdvancedDrives) // flash
				continue;

			if(pDrvive->GetDriveType() == DEVICE_HARDISK0_SYSEXT && !bShowAdvancedDrives) // sysext
				continue;

			if(pDrvive->GetDriveType() == DEVICE_HARDISK0_SYSPART && !bShowAdvancedDrives) // hddx
				continue;

			vecRetVal->push_back(m_vecDrives[x]);
		}
		//Drive* dr = m_Drives[x];
		//dr->Mount();
	}
}

CStdString CDrivesManager::getExecutablePathFromSystemPath(CStdString ExecutableSystemPath)
{
	CStdString strRet = "";
	CDrive* pDrive = getDriveByExecutableSystemPath(ExecutableSystemPath);
	
	if(pDrive == NULL)
		return "";

	ExecutableSystemPath.replace(0, pDrive->getSystemPath().length(), pDrive->getRootPath());
	strRet = CStringUtils::ReplaceAllA(ExecutableSystemPath, "\\\\", "\\");

	return CStringUtils::MakeLowercase(strRet);
}

CDrive* CDrivesManager::getDriveByExecutableSystemPath(CStdString strExecutableSystemPath)
{
	strExecutableSystemPath = CStringUtils::MakeLowercase(strExecutableSystemPath);
	CDrive *pResult = NULL;
	size_t found;
	
	for(unsigned int x = 0; x < m_vecDrives.size(); x++)
	{
		found = strExecutableSystemPath.find(CStringUtils::MakeLowercase(m_vecDrives[x]->getSystemPath()));
		if(found != strExecutableSystemPath.npos)
		{
			pResult = m_vecDrives[x];
			break;
		}
	}
	
	return pResult;
}
#include "Drive.h"
#include "utils\log.h"
#include "utils\StringUtils.h"
#include "..\xbox\XBKernalExports.h"
#include "utils\Util.h"

CDrive::CDrive(CStdString strMountPoint, CStdString strSystemPath, eDriveType DriveType)
{
	m_strMountPoint = strMountPoint;
	m_strSystemPath = strSystemPath;
	m_DriveType = DriveType;
}

CDrive::~CDrive()
{
}

HRESULT CDrive::Mount()
{
	// Get our serial
	LoadSerial();

	// Get our serial as a string
	CHAR serialStr[41]; serialStr[40] = 0; UINT outLen = 0x40;
	GetBytesString(m_Serial, 0x14, serialStr, &outLen);
	m_StrSerial = (CStdString)serialStr;

	if (m_DriveType == DEVICE_USBMEMORY_UNIT0 || 
		m_DriveType == DEVICE_USBMEMORY_UNIT1 ||
		m_DriveType == DEVICE_USBMEMORY_UNIT2 )
		m_StrSerial = m_StrSerial + "_USBMU";

	if (m_DriveType == DEVICE_USBMEMORY_Cache0 ||
		m_DriveType == DEVICE_USBMEMORY_Cache1 ||
		m_DriveType == DEVICE_USBMEMORY_Cache2 )
		m_StrSerial = m_StrSerial + "_USBMUC";

	CLog::Log(LOGNOTICE, "Mounting %s", m_strMountPoint.c_str());

	char MountConv[260];
	sprintf_s(MountConv, "\\??\\%s", m_strMountPoint.c_str());
	char SysPath[260];

	sprintf_s(SysPath, "%s", m_strSystemPath.c_str());

	STRING sSysPath = { (USHORT)strlen( SysPath ), (USHORT)strlen( SysPath ) + 1, SysPath };
	STRING sMountConv = { (USHORT)strlen( MountConv ), (USHORT)strlen( MountConv ) + 1, MountConv };
	
	// Don't try to mount 'Game:' as it's already there
	if(stricmp(m_strMountPoint.c_str(), "Game:") != 0)
	{
		int iRes = ObCreateSymbolicLink(&sMountConv, &sSysPath);
	
		if(iRes != 0)
		{
//			CLog::Log(LOGERROR, "Mount failed : %d", iRes);
			return iRes;
		}
	}

	HRESULT Result = IsMounted();

	if(Result == 1)
		GetDiskFreeSpaceEx(getRootPath().c_str(), &m_FreeBytesAvailable, 
		&m_TotalNumberOfBytes, &m_TotalNumberOfFreeBytes);
	else
	{
		m_FreeBytesAvailable.QuadPart = 0;
		m_TotalNumberOfBytes.QuadPart = 0;
		m_TotalNumberOfFreeBytes.QuadPart = 0;
	}

	return Result;
}

bool CDrive::IsMounted()
{
	using XFILE::CFile;
	// MATTIE: changed mounted test to also report empty drives as 
	// mounted, instead of searching for a file on a drive. 
	bool bResult = false; 
	
	if (CUtil::FileExists(m_strMountPoint + "\\")) 
		bResult = true;

	return bResult;
}

CStdString CDrive::GetCleanDriveName()
{
	return CStringUtils::ReplaceAllA(m_strMountPoint, ":", "");
}

HRESULT CDrive::Unmount()
{
	char MountConv[16];
	sprintf_s(MountConv, "\\??\\%s", m_strMountPoint.c_str());

	STRING sMountConv = { (USHORT)strlen(MountConv), (USHORT)strlen(MountConv) + 1, MountConv };
	return (HRESULT) ObDeleteSymbolicLink(&sMountConv);
}

bool CDrive::LoadSerial()
{
	// Clear our serial
	ZeroMemory(m_Serial, 0x14);

	if(m_DriveType == DEVICE_CDROM0)
	{
		memcpy(m_Serial, "DVDROM", 6);
		return true;
	}

	// Make sure we can get a serial for this
	if( m_DriveType != DEVICE_NAND_FLASH &&
		m_DriveType != DEVICE_MEMORY_UNIT0 && m_DriveType != DEVICE_MEMORY_UNIT1 &&
		m_DriveType != DEVICE_MEMORY_ONBOARD && m_DriveType != DEVICE_USB0 &&
		m_DriveType != DEVICE_USB1 && m_DriveType != DEVICE_USB2 &&
		m_DriveType != DEVICE_HARDISK0_PART1 && m_DriveType != DEVICE_HARDISK0_SYSPART &&
		m_DriveType != DEVICE_USBMEMORY_UNIT0 && m_DriveType != DEVICE_USBMEMORY_UNIT1 && m_DriveType != DEVICE_USBMEMORY_UNIT2 &&
		m_DriveType != DEVICE_USBMEMORY_Cache0 && m_DriveType != DEVICE_USBMEMORY_Cache1 && m_DriveType != DEVICE_USBMEMORY_Cache2)
		return true;

	// Check if it is our flash
	if(m_DriveType == DEVICE_NAND_FLASH)
	{
		BYTE xboxSerial[0x0C]; DWORD serialLength = 0x0C;
		if(XeKeysGetKey(0x14, xboxSerial, &serialLength) != 0)
			return false;
		
		XeCryptSha(xboxSerial, 0x0C, NULL, 0, NULL, 0, m_Serial, 0x14);
			return true;
	}

	// Lets open our device
	HANDLE fHandle = NULL;	STRING devicePath; IO_STATUS_BLOCK ioStatus;
	RtlInitAnsiString(&devicePath, m_strSystemPath.c_str());
	OBJECT_ATTRIBUTES objAtt = {NULL, &devicePath, OBJ_CASE_INSENSITIVE};
	
	if(NtOpenFile(&fHandle, -0x7FF0, &objAtt, &ioStatus, 3, 0x20) != STATUS_SUCCESS)
		return false;

	// Figure out our control code
	ULONG ioCtrlCode = 0;
	switch(m_DriveType)
	{
		case DEVICE_MEMORY_UNIT0:
		case DEVICE_MEMORY_UNIT1:
		case DEVICE_MEMORY_ONBOARD:
			ioCtrlCode = 0x3A0004;
			break;
		case DEVICE_USB0:
		case DEVICE_USB1:
		case DEVICE_USB2:
		case DEVICE_USBMEMORY_UNIT0:
		case DEVICE_USBMEMORY_UNIT1:
		case DEVICE_USBMEMORY_UNIT2:
		case DEVICE_USBMEMORY_Cache0:
		case DEVICE_USBMEMORY_Cache1:
		case DEVICE_USBMEMORY_Cache2:
			ioCtrlCode = 0x2D0010;
			break;
		case DEVICE_HARDISK0_PART1:
		case DEVICE_HARDISK0_SYSPART:
			ioCtrlCode = 0x4D028;
			break;
		default:
			return true;
	}

	// If its not our HDD lets just do a simple version
	if(m_DriveType != DEVICE_HARDISK0_PART1 && m_DriveType != DEVICE_HARDISK0_SYSPART)
	{
		// Get our serial 
		NTSTATUS status = NtDeviceIoControlFile(fHandle, NULL, NULL, NULL, 
			&ioStatus, ioCtrlCode, NULL, 0, m_Serial, 0x14);
		
		// Close our device and return status
		NtClose(fHandle);
		return status == STATUS_SUCCESS;
	}

	// Its a hdd so lets setup a request for our info
	BYTE ideIdentifyData[0x200];
	REQUEST_PACKET rp; ZeroMemory(&rp, 0x10);
	rp.code = 0xEC;	rp.size = 0x200;
	rp.pointer = ideIdentifyData;	

	// Now lets make the request
	NTSTATUS status = NtDeviceIoControlFile(fHandle, NULL, NULL, NULL, 
		&ioStatus, ioCtrlCode, &rp, 0x10, &rp, 0x10);
	
	// Close our device
	NtClose(fHandle);

	// If its a success lets copy in our serial
	if(status == STATUS_SUCCESS)
	{
		// Copy our serial from the HDD
		memcpy(m_Serial, ideIdentifyData + 0x14, 0x14);

		// If its the system ext lets change it so it wont conflict with partition1
		if(m_DriveType == DEVICE_HARDISK0_SYSPART)
			XeCryptSha(m_Serial, 0x14, NULL, 0, NULL, 0, m_Serial, 0x14);

		return true;
	}

	// We shouldent get here :(
	return false;
}

HRESULT CDrive::GetBytesString(BYTE* Data, UINT DataLen, CHAR* OutBuffer, UINT* OutLen)
{
	// Check our lenghts
	if(*OutLen < (DataLen * 2))
		return S_FALSE;

	*OutLen = DataLen * 2;

	// Output into our buffer as hex
	CHAR hexChars[] = "0123456789ABCDEF";

	for(UINT x = 0, y = 0; x < DataLen; x++, y+=2)
	{
		OutBuffer[y] = hexChars[(Data[x] >> 4)];
		OutBuffer[y + 1] = hexChars[(Data[x] & 0x0F)];
	}

	// All done =)
	return S_OK;
}
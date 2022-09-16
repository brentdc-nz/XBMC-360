#ifndef CDRIVE_H
#define CDRIVE_H

#include "..\utils\StdString.h"
#include "File.h"
typedef enum 
{
	DEVICE_NAND_FLASH, 
	DEVICE_MEMORY_UNIT0, 
	DEVICE_MEMORY_UNIT1,  
	DEVICE_MEMORY_ONBOARD, 
	DEVICE_CDROM0, 
	DEVICE_HARDISK0_PART1, 
	DEVICE_HARDISK0_PART0,
	DEVICE_HARDISK0_SYSPART, 
	DEVICE_HARDISK0_SYSEXT,
	DEVICE_USB0,
	DEVICE_USB1,  
	DEVICE_USB2, 
	DEVICE_HDDVD_PLAYER,         
	DEVICE_HDDVD_STORGE,
	DEVICE_TRANSFER_CABLE,
	DEVICE_TRANSFER_CABLE_SYSPART,
	DEVICE_TEST,
	DEVICE_USBMEMORY_UNIT0,
	DEVICE_USBMEMORY_UNIT1,
	DEVICE_USBMEMORY_UNIT2,
	DEVICE_USBMEMORY_Cache0,
	DEVICE_USBMEMORY_Cache1,
	DEVICE_USBMEMORY_Cache2
} eDriveType;

class CDrive
{
public:
	CDrive(CStdString strMountPoint, CStdString strSystemPath, eDriveType DriveType);
	~CDrive();

	HRESULT Mount();
	HRESULT Unmount();
	bool IsMounted();
	CStdString GetCleanDriveName();
	CStdString getRootPath() { return m_strMountPoint + "\\"; }
	eDriveType GetDriveType() { return m_DriveType; }
	CStdString getSystemPath() { return m_strSystemPath; }

private:
	bool LoadSerial();
	HRESULT GetBytesString(BYTE* Data, UINT DataLen, CHAR* OutBuffer, UINT* OutLen);

	CStdString m_strMountPoint;
	CStdString m_strSystemPath;
	eDriveType m_DriveType;
	BYTE m_Serial[0x14];
	CStdString m_StrSerial;

	ULARGE_INTEGER m_FreeBytesAvailable;
	ULARGE_INTEGER m_TotalNumberOfBytes;
	ULARGE_INTEGER m_TotalNumberOfFreeBytes;

	typedef struct _REQUEST_PACKET
	{
		DWORD unknown;
		WORD  unknown2;
		BYTE  code;
		BYTE  unknown3;
		DWORD size;
		PVOID pointer;
	} REQUEST_PACKET;
};

#endif //CDRIVE_H
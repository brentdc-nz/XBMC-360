// IoSupport.h: interface for the CIoSupport class (Xbox 360 port).

#ifndef IOSUPPORT_360_H
#define IOSUPPORT_360_H

#include "..\utils\Stdafx.h"

// Tray states
#define TRAY_OPEN						16
#define TRAY_CLOSED_NO_MEDIA		64
#define TRAY_CLOSED_MEDIA_PRESENT	96

// Drive states
#define DRIVE_OPEN					0 // Open...
#define DRIVE_NOT_READY				1 // Opening.. Closing...
#define DRIVE_READY					2
#define DRIVE_CLOSED_NO_MEDIA		3 // CLOSED...but no media in drive
#define DRIVE_CLOSED_MEDIA_PRESENT	4 // Will be send once when the drive just have closed
#define DRIVE_NONE					5 // System doesn't have an optical drive

class CIoSupport
{
public:
	static DWORD GetTrayState();
	static HRESULT ToggleTray();
	static HRESULT EjectTray();
	static HRESULT CloseTray();
};

#endif // IOSUPPORT_360_H

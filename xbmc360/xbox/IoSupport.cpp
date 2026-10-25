// Xbox 360 port of the xbmc4xbox CIoSupport tray functions.

//   EjectTray: HalWriteSMBusValue(0x20, 0x0C, FALSE, 0);  // eject tray
//   CloseTray: HalWriteSMBusValue(0x20, 0x0C, FALSE, 1);  // close tray
//   GetTrayState: HalReadSMCTrayState(&dwTrayState, &dwTrayCount)

#include "IoSupport.h"
#include "..\utils\Log.h"

// DVD_TRAY_STATE returned by XamLoaderGetDvdTrayState (xkelib xam/_xamext.h)
#define XBOX360_TRAY_EMPTY		0 // Tray closed, no media
#define XBOX360_TRAY_CLOSING	1
#define XBOX360_TRAY_OPEN		2
#define XBOX360_TRAY_OPENING	3
#define XBOX360_TRAY_CLOSED		4 // Tray closed (media detection TODO)
#define XBOX360_TRAY_ERROR		5

// setTray values for HalOpenCloseODDTray. The parameter polarity is not
// publicly documented (no public homebrew call sites could be found); if
// hardware testing shows the tray moving the wrong way, swap these two.
#define XBOX360_HAL_TRAY_OPEN	1
#define XBOX360_HAL_TRAY_CLOSE	0

// Documented kernel exports (xboxkrnl.exe ordinals 405/407), both return
// 0 (STATUS_SUCCESS) on success. Used to resolve the undocumented tray
// exports at runtime so no extra import libraries are required.
extern "C"
{
	LONG NTAPI XexGetModuleHandle(char* pszModuleName, HANDLE* phModule);
	LONG NTAPI XexGetProcedureAddress(HANDLE hModule, DWORD dwOrdinal, void** ppAddress);
}

typedef VOID (NTAPI* PFN_HALOPENCLOSEODDTRAY)(DWORD setTray);
typedef DWORD (NTAPI* PFN_XAMLOADERGETDVDTRAYSTATE)();

static PFN_HALOPENCLOSEODDTRAY s_pHalOpenCloseODDTray = NULL;
static PFN_XAMLOADERGETDVDTRAYSTATE s_pXamLoaderGetDvdTrayState = NULL;
static bool s_bTrayExportsResolved = false;

// Resolves both tray exports once. ToggleTray needs both (state read to pick
// the direction), so they are treated as all-or-nothing; failure is logged
// once and every tray call degrades gracefully afterwards.
static bool ResolveTrayExports()
{
	if (s_bTrayExportsResolved)
		return (s_pHalOpenCloseODDTray != NULL && s_pXamLoaderGetDvdTrayState != NULL);

	s_bTrayExportsResolved = true;

	HANDLE hKernel = NULL;
	HANDLE hXam = NULL;
	if (XexGetModuleHandle("xboxkrnl.exe", &hKernel) == 0 && hKernel != NULL)
		XexGetProcedureAddress(hKernel, 36, (void**)&s_pHalOpenCloseODDTray);
	if (XexGetModuleHandle("xam.xex", &hXam) == 0 && hXam != NULL)
		XexGetProcedureAddress(hXam, 426, (void**)&s_pXamLoaderGetDvdTrayState);

	if (s_pHalOpenCloseODDTray == NULL || s_pXamLoaderGetDvdTrayState == NULL)
	{
		CLog::Log(LOGERROR, "%s: failed to resolve tray exports (HalOpenCloseODDTray=%p, XamLoaderGetDvdTrayState=%p)",
							__FUNCTION__, (void*)s_pHalOpenCloseODDTray, (void*)s_pXamLoaderGetDvdTrayState);
		return false;
	}
	return true;
}

DWORD CIoSupport::GetTrayState()
{
	if (!ResolveTrayExports())
		return DRIVE_NOT_READY;

	switch (s_pXamLoaderGetDvdTrayState())
	{
		case XBOX360_TRAY_OPEN:		return TRAY_OPEN; // tray fully open
		case XBOX360_TRAY_OPENING:	return DRIVE_OPEN; // opening (in motion)
		case XBOX360_TRAY_CLOSING:	return DRIVE_NOT_READY;	// closing (in motion)
		case XBOX360_TRAY_EMPTY:	return TRAY_CLOSED_NO_MEDIA; // closed, no media
		case XBOX360_TRAY_CLOSED:	return TRAY_CLOSED_NO_MEDIA; // closed (media detection needs XamLoaderGetMediaInfo, TODO)
		default:					return DRIVE_NOT_READY;	// XBOX360_TRAY_ERROR
	}
}

HRESULT CIoSupport::EjectTray()
{
	if (!ResolveTrayExports())
		return E_FAIL;

	s_pHalOpenCloseODDTray(XBOX360_HAL_TRAY_OPEN);
	return S_OK;
}

HRESULT CIoSupport::CloseTray()
{
	if (!ResolveTrayExports())
		return E_FAIL;

	s_pHalOpenCloseODDTray(XBOX360_HAL_TRAY_CLOSE);
	return S_OK;
}

HRESULT CIoSupport::ToggleTray()
{
	if (GetTrayState() == TRAY_OPEN || GetTrayState() == DRIVE_OPEN)
		return CloseTray();
	else
		return EjectTray();
}

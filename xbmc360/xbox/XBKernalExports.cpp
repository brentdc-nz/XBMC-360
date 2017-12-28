#include "XBKernalExports.h"

void CXBKernalExports::RebootXbox()
{
	HalReturnToFirmware(6);
}

void CXBKernalExports::ShutdownXbox()
{
	HalReturnToFirmware(5);
}
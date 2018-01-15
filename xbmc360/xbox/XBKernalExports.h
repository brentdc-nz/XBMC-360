#ifndef CXBKERNALEXPORTS_H
#define CXBKERNALEXPORTS_H

#include "..\utils\Stdafx.h"

typedef enum _TEMP_INDEX
{
    CPU = 0,
    GPU,
    MEM,
    BRD
} TEMP_INDEX;

//Calls to SMC message function in xboxkrnl.lib
extern "C" VOID HalReturnToFirmware(DWORD mode);
extern "C" void __stdcall HalSendSMCMessage(void* input, void* output);

class CXBKernalExports
{
public:
	static void RebootXbox();
	static void ShutdownXbox();
	static int QueryTemps(TEMP_INDEX sTempIndex);
};

#endif //CXBKERNALEXPORTS_H
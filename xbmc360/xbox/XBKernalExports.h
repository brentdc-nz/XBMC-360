#ifndef CXBKERNALEXPORTS_H
#define CXBKERNALEXPORTS_H

#include "..\utils\stdafx.h"

//Calls to SMC message function in xboxkrnl.lib
extern "C" VOID HalReturnToFirmware(DWORD mode);
extern "C" void __stdcall HalSendSMCMessage(void* input, void* output);

class CXBKernalExports
{
public:
	static void RebootXbox();
	static void ShutdownXbox();
};

#endif //CXBKERNALEXPORTS_H
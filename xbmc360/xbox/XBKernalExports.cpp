#include "XBKernalExports.h"

void CXBKernalExports::RebootXbox()
{
	HalReturnToFirmware(6);
}

void CXBKernalExports::ShutdownXbox()
{
	HalReturnToFirmware(5);
}

int CXBKernalExports::QueryTemps(TEMP_INDEX sTempIndex)
{
	// This is used to store our SMC data
	unsigned char m_SMCMessage[16];
	unsigned char m_SMCReturn[16];

 	// Zero our memory
	ZeroMemory( m_SMCMessage, sizeof(m_SMCMessage) );
	ZeroMemory( m_SMCReturn, sizeof(m_SMCReturn) );

    m_SMCMessage[0] = 0x07;
    HalSendSMCMessage(m_SMCMessage, m_SMCReturn);
	
	return (int)((m_SMCReturn[sTempIndex * 2 + 1] | (m_SMCReturn[sTempIndex * 2 +2] <<8)) / 256.0);
}
#include "DVDFactoryInputStream.h"
#include "DVDInputStream.h"
#include "DVDInputStreamFile.h"

CDVDInputStream* CDVDFactoryInputStream::CreateInputStream(IDVDPlayer* pPlayer, const char* strFile)
{
	// TODO: Only have file atm..

	return (new CDVDInputStreamFile());
}

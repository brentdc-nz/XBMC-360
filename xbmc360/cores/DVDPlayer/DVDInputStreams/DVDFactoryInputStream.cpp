#include "../../../utils/stdafx.h"
#include "DVDFactoryInputStream.h"
#include "DVDInputStream.h"
#include "DVDInputStreamFile.h"

CDVDInputStream* CDVDFactoryInputStream::CreateInputStream(IDVDPlayer* pPlayer, const std::string& file, const std::string& content)
{
	// TODO: Only have file atm..

  
  // Our file interface handles all these types of streams
  return (new CDVDInputStreamFile());
}
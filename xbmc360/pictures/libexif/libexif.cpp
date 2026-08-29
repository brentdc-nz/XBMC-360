#ifndef _LINUX
#include <xtl.h>   // Xbox 360 XDK port: no windows.h
#else
#include <memory.h>
#endif
#include "JpegParse.h"
#include "libexif.h"

#ifdef __cplusplus
extern "C" {
#endif

bool process_jpeg(const char *filename, ExifInfo_t *exifInfo, IPTCInfo_t *iptcInfo)
{
  if (!exifInfo || !iptcInfo) return false;
  CJpegParse jpeg;
  memset(exifInfo, 0, sizeof(ExifInfo_t));
  memset(iptcInfo, 0, sizeof(IPTCInfo_t));
  if (jpeg.Process(filename))
  {
    memcpy(exifInfo, jpeg.GetExifInfo(), sizeof(ExifInfo_t));
    memcpy(iptcInfo, jpeg.GetIptcInfo(), sizeof(IPTCInfo_t));
    return true;
  }
  return false;
}
#ifdef __cplusplus
}
#endif

#if 0 // test main removed for the Xbox 360 static build
#ifndef _DLL
int main(int argc, char* argv[])
{
  ExifInfo_t exifInfo;
  IPTCInfo_t iptcInfo;
  process_jpeg("D:\\(Backup)\\test_images\\PIC_ 081.jpg", &exifInfo, &iptcInfo);
	return 0;
}
#endif
#endif

# XBMC-360

This is a port of XBMC4XBOX to XBox 360 using the Microsoft XBox 360 SDK (not using Linux and Kodi).

This is a work in progress, still many features to bring over. Though getting more usable.

### Current Status:
- GUILib - Mostly ported now, still have some areas to finish
- VideoPlayer/DVDPlayer
- PAPlayer - Works well for flac and MP3 (more codecs coming)
- Python 3 - Working well, but missing some of the modules (coming soon).

Usable but many features still need reimplementation or improvements.

### Compiled build (last updated 30-08-26):
https://drive.google.com/file/d/1KIRtkuLLYL4aaWBZyKw9xJHprkBlcqKB/view

### Libraries
- FFmpeg (DVDPlayer)
- LibSMB2
- FreeType
- MilkDrop2 (Music Visualizations)
- libFlac (PAPlayer)
- libMad (PAPlayer)
- libCurl 
- libUPnP - DLNA (Neptune & Platinum)
- sqlite3
- libiconv (Charset Converter)
- libjpeg
- libid3tag
- libmicrohttpd
- libPython

### Services
- FTP Server
- NTP Client
- UPnP

### Tools
- XBMC360Tex (XPR skin textures bundle tool)

### Notes:
Special thanks to XBMC4XBOX and Kodi.

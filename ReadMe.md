# XBMC-360 Alpha

This is a port of XBMC4XBOX to XBox 360 using the Microsoft XBox 360 SDK (not using Linux and Kodi).

This is early stages and still very much a work in progress.

### Current Status:
- GUILib - The main controls are working but still have some missing.
- VideoPlayer/DVDPlayer
- PAPlayer - Works well for flac, MP3 missing timing info (coming soon and more codecs)

Usable but many features still need reimplementation or improvements.

### Compiled alpha build (last updated 25-04-26):
https://drive.google.com/file/d/1MUD0dy-XxdDGwzfQD1qQHvzCSOV8bk4w/view

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

### Services
- FTP Server
- NTP Client
- UPnP

### Notes:
Special thanks to XBMC4XBOX and Kodi.

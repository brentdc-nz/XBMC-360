# XBMC-360 Alpha

This is a port of XBMC4XBOX to XBox 360 using the Microsoft XBox 360 SDK (not using Linux and Kodi).

This is early stages and still very much a work in progress.

### Current Status:
- GUILib - Mostly ported now, still have some areas to finish
- VideoPlayer/DVDPlayer
- PAPlayer - Works well for flac and MP3 (more codecs coming)

Usable but many features still need reimplementation or improvements.

### Compiled alpha build (last updated 30-04-26):
https://drive.google.com/file/d/1CHYLyJ6rbznQSBduTT9FsRnJBXhQxxNR/view

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

### Services
- FTP Server
- NTP Client
- UPnP

### Notes:
Special thanks to XBMC4XBOX and Kodi.

import xbmc

xbmc.log("autoexec.py: Script started!", xbmc.LOGINFO)
xbmc.log("autoexec.py: Python is working on Xbox 360!", xbmc.LOGINFO)

# Test basic Python functionality
result = 2 + 2
xbmc.log("autoexec.py: 2 + 2 = %d" % result, xbmc.LOGINFO)

# Test string formatting
xbmc.log("autoexec.py: Hello from Python %s" % str(tuple([3,4,10])), xbmc.LOGINFO)

xbmc.log("autoexec.py: Script finished successfully.", xbmc.LOGINFO)

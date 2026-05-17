@echo off
setlocal

:: XBMC-360 Build Script
:: Usage: Build.bat [Debug|Release]
:: If no argument given, prompts for selection.

set PLATFORM=Xbox 360

:: Handle command-line argument or prompt
if /i "%~1"=="Debug" (
    set CONFIG=Debug
    goto :config_set
)
if /i "%~1"=="Release" (
    set CONFIG=Release
    goto :config_set
)
if not "%~1"=="" (
    echo ERROR: Invalid configuration "%~1". Use Debug or Release.
    exit /b 1
)

echo Select build configuration:
echo   [1] Debug
echo   [2] Release
echo.
set /p CHOICE="Enter choice (1 or 2): "

if "%CHOICE%"=="1" (
    set CONFIG=Debug
) else if "%CHOICE%"=="2" (
    set CONFIG=Release
) else (
    echo ERROR: Invalid choice. Enter 1 or 2.
    exit /b 1
)

:config_set

:: Try to find MSBuild
set MSBUILD=
if exist "%ProgramFiles(x86)%\MSBuild\14.0\Bin\MSBuild.exe" (
    set "MSBUILD=%ProgramFiles(x86)%\MSBuild\14.0\Bin\MSBuild.exe"
) else if exist "%ProgramFiles(x86)%\MSBuild\12.0\Bin\MSBuild.exe" (
    set "MSBUILD=%ProgramFiles(x86)%\MSBuild\12.0\Bin\MSBuild.exe"
) else if exist "%ProgramFiles(x86)%\MSBuild\4.0\Bin\MSBuild.exe" (
    set "MSBUILD=%ProgramFiles(x86)%\MSBuild\4.0\Bin\MSBuild.exe"
) else if exist "%SystemRoot%\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe" (
    set "MSBUILD=%SystemRoot%\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe"
)

if "%MSBUILD%"=="" (
    echo ERROR: MSBuild not found. Ensure Visual Studio or .NET Framework is installed.
    exit /b 1
)

echo ============================================
echo  XBMC-360 %CONFIG% Build
echo  MSBuild: %MSBUILD%
echo  Config:  %CONFIG%^|%PLATFORM%
echo ============================================
echo.

:: Track errors
set FAILED=0

:: --- Libraries ---

echo [1/13] Building ffmpeg...
"%MSBUILD%" "libraries\ffmpeg\vcproj\ffmpeg_12_msvc.sln" /p:Configuration=%CONFIG% /p:Platform="%PLATFORM%" /m /nologo /v:minimal
if errorlevel 1 (
    echo FAILED: ffmpeg
    set FAILED=1
)

echo.
echo [2/13] Building freetype...
"%MSBUILD%" "libraries\freetype\freetype.sln" /p:Configuration=%CONFIG% /p:Platform="%PLATFORM%" /m /nologo /v:minimal
if errorlevel 1 (
    echo FAILED: freetype
    set FAILED=1
)

echo.
echo [3/13] Building libcurl...
"%MSBUILD%" "libraries\libcurl\libcurl.sln" /p:Configuration=%CONFIG% /p:Platform="%PLATFORM%" /m /nologo /v:minimal
if errorlevel 1 (
    echo FAILED: libcurl
    set FAILED=1
)

echo.
echo [4/13] Building libsmb2...
"%MSBUILD%" "libraries\libsmb2\libsmb2.sln" /p:Configuration=%CONFIG% /p:Platform="%PLATFORM%" /m /nologo /v:minimal
if errorlevel 1 (
    echo FAILED: libsmb2
    set FAILED=1
)

echo.
echo [5/13] Building libPlatinum (UPnP)...
"%MSBUILD%" "libraries\libUPnP\libPlatinum.sln" /p:Configuration=%CONFIG% /p:Platform="%PLATFORM%" /m /nologo /v:minimal
if errorlevel 1 (
    echo FAILED: libPlatinum
    set FAILED=1
)

echo.
echo [6/13] Building sqlite3...
"%MSBUILD%" "libraries\sqlite3\sqlite3.sln" /p:Configuration=%CONFIG% /p:Platform="%PLATFORM%" /m /nologo /v:minimal
if errorlevel 1 (
    echo FAILED: sqlite3
    set FAILED=1
)

echo.
echo [7/13] Building libiconv...
"%MSBUILD%" "libraries\libiconv\libiconv.sln" /p:Configuration=%CONFIG% /p:Platform="%PLATFORM%" /m /nologo /v:minimal
if errorlevel 1 (
    echo FAILED: libiconv
    set FAILED=1
)

echo.
echo [8/13] Building libid3tag...
"%MSBUILD%" "libraries\libid3tag\libid3tag.sln" /p:Configuration=%CONFIG% /p:Platform="%PLATFORM%" /m /nologo /v:minimal
if errorlevel 1 (
    echo FAILED: libid3tag
    set FAILED=1
)

:: --- Sources ---

echo.
echo [9/13] Building libjpeg...
"%MSBUILD%" "libraries\libjpeg\libjpeg.sln" /p:Configuration=%CONFIG% /p:Platform="%PLATFORM%" /m /nologo /v:minimal
if errorlevel 1 (
    echo FAILED: libjpeg
    set FAILED=1
)

echo.
echo [10/13] Building libmad...
"%MSBUILD%" "sources\PAPlayer\libmad\libmad.sln" /p:Configuration=%CONFIG% /p:Platform="%PLATFORM%" /m /nologo /v:minimal
if errorlevel 1 (
    echo FAILED: libmad
    set FAILED=1
)

echo.
echo [11/13] Building libFlac...
"%MSBUILD%" "sources\PAPlayer\libFlac\libFlac.sln" /p:Configuration=%CONFIG% /p:Platform="%PLATFORM%" /m /nologo /v:minimal
if errorlevel 1 (
    echo FAILED: libFlac
    set FAILED=1
)

echo.
echo [12/13] Building 360MilkDrop2...
"%MSBUILD%" "sources\360MilkDrop2\360MilkDrop2.sln" /p:Configuration=%CONFIG% /p:Platform="%PLATFORM%" /m /nologo /v:minimal
if errorlevel 1 (
    echo FAILED: 360MilkDrop2
    set FAILED=1
)

:: --- Main Project ---

echo.
echo [13/13] Building xbmc360...
"%MSBUILD%" "xbmc360.sln" /p:Configuration=%CONFIG% /p:Platform="%PLATFORM%" /m /nologo /v:minimal
if errorlevel 1 (
    echo FAILED: xbmc360
    set FAILED=1
)

:: --- Summary ---

echo.
echo ============================================
if %FAILED%==1 (
    echo  BUILD COMPLETED WITH ERRORS
    echo ============================================
    exit /b 1
) else (
    echo  BUILD SUCCESSFUL
    echo ============================================
    exit /b 0
)

@echo off
setlocal

:: XBMC-360 Full Release Build Script
:: Builds all libraries and the main project in Release|Xbox 360

set CONFIG=Release
set PLATFORM=Xbox 360

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
echo  XBMC-360 Release Build
echo  MSBuild: %MSBUILD%
echo  Config:  %CONFIG%^|%PLATFORM%
echo ============================================
echo.

:: Track errors
set FAILED=0

:: --- Libraries ---

echo [1/9] Building ffmpeg...
"%MSBUILD%" "libraries\ffmpeg\vcproj\ffmpeg_12_msvc.sln" /p:Configuration=%CONFIG% /p:Platform="%PLATFORM%" /m /nologo /v:minimal
if errorlevel 1 (
    echo FAILED: ffmpeg
    set FAILED=1
)

echo.
echo [2/9] Building freetype...
"%MSBUILD%" "libraries\freetype\freetype.sln" /p:Configuration=%CONFIG% /p:Platform="%PLATFORM%" /m /nologo /v:minimal
if errorlevel 1 (
    echo FAILED: freetype
    set FAILED=1
)

echo.
echo [3/9] Building libcurl...
"%MSBUILD%" "libraries\libcurl\libcurl.sln" /p:Configuration=%CONFIG% /p:Platform="%PLATFORM%" /m /nologo /v:minimal
if errorlevel 1 (
    echo FAILED: libcurl
    set FAILED=1
)

echo.
echo [4/9] Building libsmb2...
"%MSBUILD%" "libraries\libsmb2\libsmb2.sln" /p:Configuration=%CONFIG% /p:Platform="%PLATFORM%" /m /nologo /v:minimal
if errorlevel 1 (
    echo FAILED: libsmb2
    set FAILED=1
)

echo.
echo [5/9] Building libPlatinum (UPnP)...
"%MSBUILD%" "libraries\libUPnP\libPlatinum.sln" /p:Configuration=%CONFIG% /p:Platform="%PLATFORM%" /m /nologo /v:minimal
if errorlevel 1 (
    echo FAILED: libPlatinum
    set FAILED=1
)

:: --- Sources ---

echo.
echo [6/9] Building libmad...
"%MSBUILD%" "sources\PAPlayer\libmad\libmad.sln" /p:Configuration=%CONFIG% /p:Platform="%PLATFORM%" /m /nologo /v:minimal
if errorlevel 1 (
    echo FAILED: libmad
    set FAILED=1
)

echo.
echo [7/9] Building libFlac...
"%MSBUILD%" "sources\PAPlayer\libFlac\libFlac.sln" /p:Configuration=%CONFIG% /p:Platform="%PLATFORM%" /m /nologo /v:minimal
if errorlevel 1 (
    echo FAILED: libFlac
    set FAILED=1
)

echo.
echo [8/9] Building 360MilkDrop2...
"%MSBUILD%" "sources\360MilkDrop2\360MilkDrop2.sln" /p:Configuration=%CONFIG% /p:Platform="%PLATFORM%" /m /nologo /v:minimal
if errorlevel 1 (
    echo FAILED: 360MilkDrop2
    set FAILED=1
)

:: --- Main Project ---

echo.
echo [9/9] Building xbmc360...
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

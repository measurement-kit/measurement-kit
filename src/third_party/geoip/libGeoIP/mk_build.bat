@echo off
if "%VSCMD_ARG_HOST_ARCH%" == "x64" (
    set MK_WINTYPE=win64
)
if "%VSCMD_ARG_HOST_ARCH%" == "x86" (
    set MK_WINTYPE=win32
)
if "%VSCMD_ARG_HOST_ARCH%" == "" (
    echo "Not in a Visual Studio Command Prompt"
    exit 1
)
del *.dll *.lib *.obj
nmake -f Makefile.vc
mkdir ..\..\..\..\build\%MK_WINTYPE%\bin
copy GeoIP.dll ..\..\..\..\build\%MK_WINTYPE%\bin
mkdir ..\..\..\..\build\%MK_WINTYPE%\include
copy GeoIP.h ..\..\..\..\build\%MK_WINTYPE%\include
copy GeoIPCity.h ..\..\..\..\build\%MK_WINTYPE%\include
mkdir ..\..\..\..\build\%MK_WINTYPE%\lib
copy GeoIP.lib ..\..\..\..\build\%MK_WINTYPE%\lib

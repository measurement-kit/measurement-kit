rem Adapted from: https://github.com/mypaint/libmypaint/commit/553f1753
rem
rem Matrix-driven Appveyor CI script for libmypaint
rem Currently only does MSYS2 builds.
rem https://www.appveyor.com/docs/installed-software#mingw-msys-cygwin
rem Needs the following vars:
rem    MSYS2_ARCH:  x86_64 or i686
rem    MSYSTEM:  MINGW64 or MINGW32

rem Set the paths appropriately
PATH C:\msys64\%MSYSTEM%\bin;C:\msys64\usr\bin;%PATH%

rem Upgrade the MSYS2 platform (double `yy` and `uu` _are not_ typos)
bash -lc "pacman --noconfirm -Syy pacman"
bash -lc "pacman --noconfirm -Syyuu"

rem Install required tools
bash -xlc "pacman --noconfirm -S --needed base-devel"
bash -xlc "pacman --noconfirm -S --needed mingw-w64-x86_64-toolchain"
bash -xlc "pacman --noconfirm -S --needed libreadline"

rem Invoke subsequent bash in the build tree
cd %APPVEYOR_BUILD_FOLDER%
set CHERE_INVOKING=yes

rem Build/test scripting
bash -xlc "./.ci/appveyor/msys2.sh"

PATH C:\msys64\%MSYSTEM%\bin;C:\msys64\usr\bin;%PATH%

cd %APPVEYOR_BUILD_FOLDER% || exit /b
set CHERE_INVOKING=yes

bash.exe -lc "curl -LsO https://github.com/measurement-kit/prebuilt/releases/download/testing/windows-geoip-api-c-1.6.12-2-g204cc59-4.tar.gz" || exit /b
cmake.exe -E tar -xzf windows-geoip-api-c-1.6.12-2-g204cc59-4.tar.gz || exit /b
bash.exe -lc "curl -LsO https://github.com/measurement-kit/prebuilt/releases/download/testing/windows-libevent-2.1.8-4.tar.gz" || exit /b
cmake.exe -E tar -xzf windows-libevent-2.1.8-4.tar.gz || exit /b
bash.exe -lc "curl -LsO https://github.com/measurement-kit/prebuilt/releases/download/testing/windows-libressl-2.6.4-4.tar.gz" || exit /b
cmake.exe -E tar -xzf windows-libressl-2.6.4-4.tar.gz || exit /b

bash.exe -lc "./autogen.sh --cmake" || exit /b
cmake.exe "-G%CMAKE_GENERATOR%" -DCMAKE_BUILD_TYPE=Release -DMK_GEOIP=MK_DIST/windows/geoip-api-c/1.6.12-2-g204cc59-4/%WINDOWS_ARCH_NAME%/ -DMK_LIBEVENT=MK_DIST/windows/libevent/2.1.8-4/%WINDOWS_ARCH_NAME%/ -DMK_LIBRESSL=MK_DIST/windows/libressl/2.6.4-4/%WINDOWS_ARCH_NAME%/ . || exit /b
cmake.exe --build . || exit /b
ctest.exe -a -j8 || exit /b

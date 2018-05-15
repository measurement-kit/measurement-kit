PATH C:\msys64\%MSYSTEM%\bin;C:\msys64\usr\bin;%PATH%

cd %APPVEYOR_BUILD_FOLDER% || exit /b
set CHERE_INVOKING=yes

bash.exe -lc "./autogen.sh --cmake" || exit /b
cmake.exe "-G%CMAKE_GENERATOR%" -DCMAKE_BUILD_TYPE=Release -DMK_DOWNLOAD_DEPS=ON . || exit /b
cmake.exe --build . -- /nologo /property:Configuration=Release || exit /b
ctest.exe -a -j8 --output-on-failure || exit /b

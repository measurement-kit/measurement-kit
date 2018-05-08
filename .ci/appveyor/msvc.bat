cd %APPVEYOR_BUILD_FOLDER%
set CHERE_INVOKING=yes
bash -lc "./autogen.sh --cmake" || exit /b
cmake.exe "-G$%CMAKE_GENERATOR%" -DCMAKE_BUILD_TYPE=Release . || exit /b
cmake.exe --build . || exit /b
ctest.exe -a -j8 || exit /b

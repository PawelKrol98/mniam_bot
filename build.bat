REM Dopasuj poniższe ścieżki:
set MINGW_PATH=C:\mingw-w64\mingw32\bin
set CMAKE_PATH=C:\CMake\bin


set PATH=%PATH%;%MINGW_PATH%;%CMAKE_PATH%;

cmake -G "MinGW Makefiles" -B build
mingw32-make.exe -C build
call setenv.bat
cd ..
cd msvs
mkdir freetype
cd freetype
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/freetype.cmake" "../../3rdparty/freetype-2.8.1"
cmake -LA -N >cfg.txt
@%comspec% /k
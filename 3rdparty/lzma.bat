call setenv.bat
cd ..
cd msvs
mkdir lzma
cd lzma
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/lzma.cmake" "../../3rdparty/xz-5.2.4"
cmake -LA -N >cfg.txt
@%comspec% /k
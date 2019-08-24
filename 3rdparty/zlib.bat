call setenv.bat
cd ..
cd msvs
mkdir zlib
cd zlib
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/zlib.cmake" "../../3rdparty/zlib-1.2.11"
cmake -LA -N >cfg.txt
@%comspec% /k
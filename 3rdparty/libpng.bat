call setenv.bat
cd ..
cd msvs
mkdir libpng
cd libpng
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/libpng.cmake" "../../3rdparty/libpng-1.6.35"
cmake -LA -N >cfg.txt
@%comspec% /k
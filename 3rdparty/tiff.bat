call setenv.bat
cd ..
cd msvs
mkdir tiff
cd tiff
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/tiff.cmake" "../../3rdparty/tiff-4.0.9"
cmake -LA -N >cfg.txt
@%comspec% /k
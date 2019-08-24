call setenv.bat
cd ..
cd msvs
mkdir bzip2
cd bzip2
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/bzip2.cmake" "../../3rdparty/bzip2-1.0.6"
cmake -LA -N >cfg.txt
@%comspec% /k
call setenv.bat
cd ..
cd msvs
mkdir dirent
cd dirent
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/dirent.cmake" "../../3rdparty/dirent-1.23.2"
cmake -LA -N >cfg.txt
@%comspec% /k
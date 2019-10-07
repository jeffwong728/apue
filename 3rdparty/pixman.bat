call setenv.bat
cd ..
cd msvs
mkdir pixman
cd pixman
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/pixman.cmake" "../../3rdparty/pixman-0.38.4/pixman"
cmake -LA -N >cfg.txt
@%comspec% /k
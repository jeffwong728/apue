call setenv.bat
cd ..
cd msvs
mkdir freeglut
cd freeglut
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/freeglut.cmake" "../../3rdparty/freeglut-3.0.0"
cmake -LA -N >cfg.txt
@%comspec% /k
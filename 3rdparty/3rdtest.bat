call setenv.bat
cd ..
cd msvs
mkdir 3rdtest
cd 3rdtest
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/3rdtest.cmake" "../../3rdparty/3rdtest"
cmake -LA -N . >cfg.txt
@%comspec% /k
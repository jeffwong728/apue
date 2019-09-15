call setenv.bat
cd ..
cd msvs
mkdir eigen
cd eigen
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/eigen.cmake" "../../3rdparty/eigen-3.3.7"
cmake -LA -N >cfg.txt
@%comspec% /k
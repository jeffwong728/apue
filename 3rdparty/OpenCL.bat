call setenv.bat
cd ..
cd msvs
mkdir OpenCL
cd OpenCL
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/OpenCL.cmake" "../../3rdparty/OpenCL"
cmake -LA -N >cfg.txt
@%comspec% /k
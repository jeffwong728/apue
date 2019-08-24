call setenv.bat
cd ..
cd build
mkdir simd
cd simd
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/simd.cmake" "../../3rdparty/simd-4.3.79/prj/cmake"
cmake -LA -N >cfg.txt
@%comspec% /k
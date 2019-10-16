call ..\setEnv.bat
cd ..
cd msvs
mkdir simd
cd simd
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/simd.cmake" "../../3rdparty/simd-4.4.82"
cmake -LA -N . >cfg.txt
@%comspec% /k
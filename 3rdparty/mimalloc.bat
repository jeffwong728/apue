call ..\setEnv.bat
cd ..
cd msvs
mkdir mimalloc
cd mimalloc
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/mimalloc.cmake" "../../3rdparty/mimalloc-1.1.0" -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT_DIR%\scripts\buildsystems\vcpkg.cmake
cmake -LA -N . >cfg.txt
@%comspec% /k
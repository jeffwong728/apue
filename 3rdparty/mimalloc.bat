call ..\setEnv.bat
cd ..
cd build
mkdir mimalloc
cd mimalloc
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/mimalloc.cmake" "../../3rdparty/mimalloc" -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT_DIR%\scripts\buildsystems\vcpkg.cmake
@%comspec% /k

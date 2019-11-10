call ..\setEnv.bat
cd ..
cd msvs
mkdir iplplus
cd iplplus
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/iplplus.cmake" "../../3rdparty/iplplus" -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT_DIR%\scripts\buildsystems\vcpkg.cmake
cmake -LA -N . >cfg.txt
@%comspec% /k
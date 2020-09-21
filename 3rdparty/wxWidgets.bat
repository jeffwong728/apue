call ..\setEnv.bat
cd ..
cd msvs
mkdir wxWidgets
cd wxWidgets
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/wxWidgets.cmake" "../../3rdparty/wxWidgets" -DCMAKE_TOOLCHAIN_FILE=%VCPKG_STATIC_ROOT_DIR%\scripts\buildsystems\vcpkg.cmake
cmake -LA -N . >cfg.txt
@%comspec% /k
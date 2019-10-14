call setenv.bat
cd ..
cd msvs
mkdir wxSVG
cd wxSVG
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/wxSVG.cmake" "../../3rdparty/wxsvg-1.5.21" -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT_DIR%\scripts\buildsystems\vcpkg.cmake
cmake -LA -N . >cfg.txt
@%comspec% /k
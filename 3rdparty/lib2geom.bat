call setenv.bat
cd ..
cd msvs
mkdir lib2geom
cd lib2geom
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/lib2geom.cmake" "../../3rdparty/lib2geom" -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT_DIR%\scripts\buildsystems\vcpkg.cmake
cmake -LA -N . >cfg.txt
@%comspec% /k
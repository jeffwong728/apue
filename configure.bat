call setpath.bat
cd msvs
mkdir apue
cd apue
cmake -G"Visual Studio 15 2017 Win64" "../.." -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT_DIR%\scripts\buildsystems\vcpkg.cmake
cmake -LA -N . >cfg.txt
@%comspec% /k
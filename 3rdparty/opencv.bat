call setenv.bat
cd ..
cd msvs
mkdir opencv
cd opencv
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/opencv.cmake" "../../3rdparty/opencv-4.1.1" -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT_DIR%\scripts\buildsystems\vcpkg.cmake
cmake -LA -N . >cfg.txt
@%comspec% /k
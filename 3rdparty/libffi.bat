call setenv.bat
cd ..
cd msvs
mkdir libffi
cd libffi
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/libffi.cmake" "../../3rdparty/libffi-3.3-rc0"
cmake -LA -N >cfg.txt
@%comspec% /k
call setenv.bat
cd ..
cd msvs
mkdir "libsigc++"
cd "libsigc++"
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/libsigc++.cmake" "../../3rdparty/libsigc++-2.99.11"
cmake -LA -N >cfg.txt
@%comspec% /k
call setenv.bat
cd ..
cd msvs
mkdir libiconv
cd libiconv
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/libiconv.cmake" "../../3rdparty/libiconv-1.15"
cmake -LA -N >cfg.txt
@%comspec% /k
call setenv.bat
cd ..
cd msvs
mkdir pcre
cd pcre
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/pcre.cmake" "../../3rdparty/pcre-8.43"
cmake -LA -N . >cfg.txt
@%comspec% /k
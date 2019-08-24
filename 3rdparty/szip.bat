call setenv.bat
cd ..
cd msvs
mkdir szip
cd szip
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/szip.cmake" -DCMAKE_INSTALL_PREFIX="../../install/szip" "../../3rdparty/szip-2.1.1"
cmake -LA -N >cfg.txt
@%comspec% /k
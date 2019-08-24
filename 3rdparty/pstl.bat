call setenv.bat
cd ..
cd msvs
mkdir pstl
cd pstl
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/pstl.cmake" -DTBB_DIR="../../install/tbb/cmake" -DCMAKE_INSTALL_PREFIX="../../install/pstl" "../../3rdparty/pstl2019_20180718oss"
cmake -LA -N >cfg.txt
@%comspec% /k
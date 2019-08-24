call setenv.bat
cd ..
cd msvs
mkdir hdf5
cd hdf5
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/hdf5.cmake" -DSZIP_DIR="../../../install/szip/cmake/szip" -DZLIB_ROOT="../../install/zlib" -DCMAKE_INSTALL_PREFIX="../../install/hdf5" "../../3rdparty/CMake-hdf5-1.10.3/hdf5-1.10.3"
cmake -LA -N >cfg.txt
@%comspec% /k
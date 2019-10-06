call setenv.bat
cd ..
cd msvs
mkdir libexpat
cd libexpat
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/libexpat.cmake" "../../3rdparty/libexpat-R_2_2_9/expat"
cmake -LA -N >cfg.txt
@%comspec% /k
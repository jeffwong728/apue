call setenv.bat
cd ..
cd msvs
mkdir "libjpeg-turbo"
cd "libjpeg-turbo"
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/libjpeg-turbo.cmake" "../../3rdparty/libjpeg-turbo-2.0.3"
cmake -LA -N >cfg.txt
@%comspec% /k
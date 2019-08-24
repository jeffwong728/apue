call setenv.bat
cd ..
cd msvs
mkdir opencv
cd opencv
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/opencv.cmake" "../../3rdparty/opencv-4.1.1"
cmake -LA -N >cfg.txt
@%comspec% /k
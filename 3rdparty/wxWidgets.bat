call setenv.bat
cd ..
cd msvs
mkdir wxWidgets
cd wxWidgets
cmake -G"Visual Studio 15 2017 Win64" -C"../../3rdparty/cmake/wxWidgets.cmake" "../../3rdparty/wxWidgets-3.1.2"
cmake -LA -N . >cfg.txt
@%comspec% /k
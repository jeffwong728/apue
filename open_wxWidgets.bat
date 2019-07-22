@set PATH=%PATH%;C:\Program Files\CMake\bin
@set PATH=%PATH%;D:\apue\install\glib\bin
@set PATH=%PATH%;D:\apue\install\tbb\bin\intel64\vc14
@set PATH=%PATH%;D:\apue\install\opencv\x64\vc15\bin
cd msvs
cd wxWidgets
cmake --open .
@rem %comspec% /k
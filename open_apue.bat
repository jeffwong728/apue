@set PATH=%PATH%;D:\Program Files\CMake\bin
@set PATH=%PATH%;D:\apue\install\glib\bin
@set PATH=%PATH%;D:\apue\install\tbb\bin\intel64\vc14
@set PATH=%PATH%;D:\apue\install\opencv\x64\vc15\bin
@set PATH=%PATH%;D:\apue\install\icu\bin64
cd build
cd apue
cmake --open .
@rem %comspec% /k
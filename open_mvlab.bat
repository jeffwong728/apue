@set PATH=%PATH%;C:\Program Files\CMake\bin
@set PATH=%PATH%;D:\apue\install\glib\bin
@set PATH=%PATH%;D:\apue\install\tbb\bin\intel64\vc14
@set PATH=%PATH%;D:\apue\install\opencv\x64\vc15\bin
@set PATH=%PATH%;D:\apue\install\OpenBLAS\bin
cd msvs
cd opencv
cd modules
cd mvlab
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\Common7\IDE\devenv.com" opencv_mvlab.sln
@rem %comspec% /k
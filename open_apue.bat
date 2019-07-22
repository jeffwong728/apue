@set PATH=%PATH%;C:\Program Files\CMake\bin
@set PATH=%PATH%;D:\apue\install\glib\bin
@set PATH=%PATH%;D:\apue\install\tbb\bin\intel64\vc14
@set PATH=%PATH%;D:\apue\install\opencv\x64\vc15\bin
@set PATH=%PATH%;D:\apue\install\apue\bin
@set SPAM_UNITTEST_ROOT=%~dp0spam\unittest
@set BOOST_TEST_LOG_LEVEL=message
cd build
cd apue
cmake --open .
@rem %comspec% /k
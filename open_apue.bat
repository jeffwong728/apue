call setEnv.bat
@set PATH=%VCPKG_ROOT_DIR%\installed\x64-windows\bin;%PATH%
@set SPAM_UNITTEST_ROOT=%~dp0spam\unittest
@set BOOST_TEST_LOG_LEVEL=message
@set BOOST_TEST_DETECT_MEMORY_LEAK=0
cd msvs
cd apue
cmake --open .
@rem %comspec% /k
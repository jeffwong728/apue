call setpath.bat
@set SPAM_UNITTEST_ROOT=%~dp0spam\unittest
@set BOOST_TEST_LOG_LEVEL=message
cd build
cd apue
cmake --open .
@rem %comspec% /k
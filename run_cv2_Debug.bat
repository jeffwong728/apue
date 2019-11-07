@call setEnv.bat
@set PATH=%VCPKG_ROOT_DIR%\installed\x64-windows\debug\bin;%PATH%
C:\Python37\python.exe -m unittest discover -v -s test -p "test_*.py"
@rem start "" "%TEMP%\mvlab.log"
@pause
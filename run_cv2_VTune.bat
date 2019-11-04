@call setEnv.bat
@set PATH=%VCPKG_ROOT_DIR%\installed\x64-windows\bin;%PATH%
C:\Python37\python.exe -m unittest discover -v -s test -p "test_*.py"
@start "C:\Program Files\Notepad++\notepad++.exe" "%TEMP%\mvlab.log"
@pause
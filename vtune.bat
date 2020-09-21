@call setEnv.bat
@set PATH=%VCPKG_ROOT_DIR%\installed\x64-windows\bin;%PATH%
C:\Python37\python.exe test\vtune.py
pause
@call setEnv.bat
@set PATH=%VCPKG_ROOT_DIR%\installed\x64-windows\bin;%PATH%
@rem C:\Python37\python.exe -m unittest discover -v -s test -p "test_*.py"
C:\Python37\python.exe test\run.py
@rem start "" "%TEMP%\mvlab.log"
@rem C:\Python37\python.exe test\genperfplot.py
@rem start "" "%TEMP%\mvlab-perf.svg"
@start "" reports\mvlab.html

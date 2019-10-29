call setEnv.bat
cd msvs
cd opencv
@set "VS150COMNTOOLS=%VS2017INSTALLDIR%\Common7\Tools\"
@call "%VS150COMNTOOLS%VsDevCmd.bat"

if exist "%TEMP%\cv_build.log" (
  del "%TEMP%\cv_build.log"
)

devenv OpenCV.sln /build "Debug|x64" /Project INSTALL /Out "%TEMP%\cv_build.log"
start "‪C:\Program Files\Notepad++\notepad++.exe" "%TEMP%\cv_build.log"

@rem %comspec% /k
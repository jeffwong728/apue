%~d0
cd %~dp0

del apue.zip
@rem "C:\Program Files\7-Zip\7z.exe" a -tzip apue.zip . -xr!*.obj -xr!*.pdb -xr!*.exe -xr!*.db -xr!*.lib -xr!*.exp -xr!*.ilk -xr!*.dblite -xr!*.iobj -xr!*.ipdb -xr!*.pyd -xr!*.ipch -xr!*.vs -xr!build -xr!install -xr!x64 -xr!msvs
"C:\Program Files\7-Zip\7z.exe" a -tzip apue.zip 3rdparty\cmake 3rdparty\3rdtest 3rdparty\*.bat apex boostpy cmake config pyext site_scons spam test Tutorial .\*.* -xr!spam\armadillo -xr!spam\ensmallen -xr!apex\modules\text -xr!spam\unittest\idata -xr!spam\unittest\rdata
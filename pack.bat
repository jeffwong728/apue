%~d0
cd %~dp0
set ISODATE=%date:~6,4%-%date:~0,2%-%date:~3,2%
@rem "C:\Program Files\7-Zip\7z.exe" a -tzip apue.zip . -xr!*.obj -xr!*.pdb -xr!*.exe -xr!*.db -xr!*.lib -xr!*.exp -xr!*.ilk -xr!*.dblite -xr!*.iobj -xr!*.ipdb -xr!*.pyd -xr!*.ipch -xr!*.vs -xr!build -xr!install -xr!x64 -xr!msvs
"C:\Program Files\7-Zip\7z.exe" a -tzip apue-%ISODATE%.zip 3rdparty\cmake 3rdparty\3rdtest 3rdparty\*.bat apex boostpy cmake config pyext spam test Tutorial .\*.* -xr!spam\armadillo -xr!spam\ensmallen -xr!apex\modules\text -xr!spam\unittest\idata -xr!spam\unittest\rdata -xr!apex\modules\ximgproc -xr!apex\modules\hdf -xr!test\.vs -xr!__pycache__ -xr!reports -xr!test\data
@if not [%1]==[] (set ZIPTAR=win64.zip & set SEPTAR=ibm_utl_cbbsep_1.00_win_x86_64.zip & set RunPath=win64) else (set ZIPTAR=win.zip & set SEPTAR=ibm_utl_cbbsep_1.00_win_i386.zip & set RunPath=win)
move /Y c:\IBM_Support c:\IBM_Support.bak
pkzip25.exe -extract -dir -over %ZIPTAR%
pkzip25.exe -extract -dir -over %SEPTAR%
xcopy /Y /s cbb\* %RunPath%\image\
cd %RunPath%\image\
rem set DSA_INCLUDE=hardwareinfo
rem cbbcli.exe collect -v -U 
cbbcli.exe -test deltaspec=..\..\deltaspec.xml compare=C:\BVT\compare.xml.gz output=C:\IBM_Support\DiffSummary.html
cd ..\..
rd /q /s cbb
rd /q /s %RunPath%

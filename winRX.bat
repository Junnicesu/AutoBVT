set Tar_IP=%1
set ZipTar=%2
set SepTar=%3
set Parameter=%4
cd /d D:\AutoBVT\remoteux_win\
rX.exe %Tar_IP% administrator SYS2009health C:\ "C:\winBVTCasesOnRt.bat  %Parameter%" -t ..\pkzip25.exe ..\winBVTCasesOnRt.bat ..\deltaspec.xml %zipTar% %sepTar% -b C:\IBM_Support\
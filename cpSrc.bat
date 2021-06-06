set SRCBASE_PATH=%1
set CBB_LEVEL=%2
set SRC_DIR=%3
set TARGET=%4
robocopy \\bcrfss03.raleigh.ibm.com\SRCBASE\%SRCBASE_PATH%\%CBB_LEVEL%  %SRC_DIR%  %TARGET% /ETA /XO %5 %6 %7 %8 %9 

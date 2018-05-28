@echo off
:: 如果要文件名带上路径，则需要在dir这一句的%%~nxi上作改动
::                  code by FBY && RMW
if exist bmpFileNameList.txt del bmpFileNameList.txt /q
::for /f "delims=" %%i in ('dir *.bmp /b /a-d /s') do echo %%~nxi >>bmpFileNameList.txt
for /f "delims=" %%i in ('dir *.bmp /b /a-d /s') do echo %%~dpi%%~nxi >>bmpFileNameList.txt
if not exist bmpFileNameList.txt goto no_file
start bmpFileNameList.txt
exit

:no_file
cls
echo       %cur_dir% 文件夹下没有单独的文件
pause 
@echo off
:: ���Ҫ�ļ�������·��������Ҫ��dir��һ���%%~nxi�����Ķ�
::                  code by FBY && RMW
if exist bmpFileNameList.txt del bmpFileNameList.txt /q
::for /f "delims=" %%i in ('dir *.bmp /b /a-d /s') do echo %%~nxi >>bmpFileNameList.txt
for /f "delims=" %%i in ('dir *.bmp /b /a-d /s') do echo %%~dpi%%~nxi >>bmpFileNameList.txt
if not exist bmpFileNameList.txt goto no_file
start bmpFileNameList.txt
exit

:no_file
cls
echo       %cur_dir% �ļ�����û�е������ļ�
pause 
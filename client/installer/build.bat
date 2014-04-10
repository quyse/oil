@echo off
call tools\setenv.bat

rem get version
set /p version=<version.txt

echo ======================= make content.jar

erase /Q out\*
7z a -tzip -mx=9 -mcu=on out\content.jar ..\chrome\content


echo ======================= make application.ini

call :appini > out\application.ini


echo ======================= make windows installer package

call :heatdir ..\..\assets assets APPLICATIONFOLDER
call :heatdir tools\xulrunner xulrunner APPLICATIONFOLDER

candle -nologo oilclient.wxs -out out\oilclient.wixobj -dversion=%version%

light -nologo -ext WixUIExtension -cultures:en-us -out C:\tmp\oilclient_%version%.msi out\oilclient.wixobj out\assets.wixobj out\xulrunner.wixobj

goto :eof


rem === functions

:heatdir
heat dir %1 -nologo -sfrag -scom -sreg -gg -g1 -cg %2 -dr %3 -var var.%2_source -out out\%2.wxs
candle -nologo out\%2.wxs -out out\%2.wixobj -d%2_source=%1
goto :eof

:appini
echo Vendor=quyse.net
echo Name=oil
echo Version=%version%
echo BuildID=1234567
echo ID=oil@quyse.net
echo [Gecko]
echo MinVersion=1.8
echo MaxVersion=200.*
goto :eof

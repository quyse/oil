@echo off
call tools\setenv.bat

echo ======================= make content.jar

erase /Q out\*
7z a -tzip -mx=9 -mcu=on out\content.jar ..\chrome\content


echo ======================= make windows installer package

call :heatdir ..\..\assets assets APPLICATIONFOLDER
call :heatdir tools\xulrunner xulrunner APPLICATIONFOLDER

candle -nologo oilclient.wxs -out out\oilclient.wixobj

light -nologo -ext WixUIExtension -cultures:en-us -out C:\tmp\oilclient.msi out\oilclient.wixobj out\assets.wixobj out\xulrunner.wixobj

goto :eof


rem === functions

:heatdir
heat dir %1 -nologo -sfrag -scom -sreg -gg -g1 -cg %2 -dr %3 -var var.%2_source -out out\%2.wxs
candle -nologo out\%2.wxs -out out\%2.wixobj -d%2_source=%1
goto :eof

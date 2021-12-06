param([string]$application="C:\Windows\explorer.exe")
Measure-Command { ..\..\..\..\pin.exe -t ..\x64\Debug\MyPinTool.dll -o mypinlog.txt -- $application }
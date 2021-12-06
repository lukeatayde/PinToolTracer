﻿param([string]$application="C:\Users\Raul\source\repos\dll_hijacking_demo\x64\Debug\vulnerable_application")
$normal_application = Measure-Command {& $application}
$tracing = Measure-Command { ..\..\..\..\pin.exe -t ..\x64\Debug\MyPinTool.dll -o mypinlog.txt -- $application}
"Normal execution time for {0}: {1} milliseconds" -f $application, $normal_application.TotalMilliseconds
"Tracing execution time for {0}: {1} milliseconds" -f $application, $tracing.TotalMilliseconds
$tracing_overhead = (($tracing.TotalMilliseconds - $normal_application.TotalMilliseconds)/ $normal_application.TotalMilliseconds) *100
"Tracing Overhead Percentage : {0}%" -f [math]::Round($tracing_overhead,2)
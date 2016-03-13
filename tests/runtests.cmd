@echo off
for %%i in ( test*.zs ) do call :run %%i %%~ni
exit

:run
..\build\zorro %1 >last.txt
diff %2.ok last.txt
if not errorlevel 0 (
    echo %1 fail
    exit
) else (
    echo %1 ok
)
exit /B

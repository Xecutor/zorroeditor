@echo off
for %%i in ( test*.zs ) do call :run %%i %%~ni
if exist last.txt del last.txt
if exist zorro.log del zorro.log

exit

:run
  call ..\build\zorro %1 >last.txt
  call diff -w %2.ok last.txt
  if not errorlevel 0 (
      echo %1 fail
      exit
  ) else (
      echo %1 ok
  )
  exit /B

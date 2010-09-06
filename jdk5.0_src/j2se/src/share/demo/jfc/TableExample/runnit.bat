@echo off
rem Run the TableExample demo
rem
rem @(#)runnit.bat	1.10 99/07/12

if "%SWING_HOME%" == "" goto nohome
if "%JDBCHOME%" == "" goto nojdbchome
@echo on
java -classpath ".;TableExample.jar;%JDBCHOME%;%SWING_HOME%;%SWING_HOME%\swing.jar;%SWING_HOME%\windows.jar;%SWING_HOME%\motif.jar;%SWING_HOME%\multi.jar;%CLASSPATH%" TableExample
@echo off
goto done

:nohome
echo No SWING_HOME environment variable set.

:nojdbchome
echo No JDBCHOME environment variable set.

:done

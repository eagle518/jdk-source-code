@echo off
rem Run the Stylepad demo
rem
rem 1.1 97/11/24

if "%SWING_HOME%" == "" goto nohome
@echo on
java -classpath ".;Stylepad.jar;%SWING_HOME%;%SWING_HOME%\swing.jar;%SWING_HOME%\windows.jar;%SWING_HOME%\motif.jar;%SWING_HOME%\multi.jar;%CLASSPATH%" Stylepad 
@echo off
goto done

:nohome
echo No SWING_HOME environment variable set.

:done

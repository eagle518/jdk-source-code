@echo off
rem Run the Notepad demo
rem
rem 1.5 98/07/19

if "%SWING_HOME%" == "" goto nohome
@echo on
java -classpath ".;Notepad.jar;%SWING_HOME%;%SWING_HOME%\swing.jar;%SWING_HOME%\windows.jar;%SWING_HOME%\motif.jar;%SWING_HOME%\multi.jar;%CLASSPATH%" Notepad 
@echo off
goto done

:nohome
echo No SWING_HOME environment variable set.

:done


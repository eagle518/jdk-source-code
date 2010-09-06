@echo off
rem Run the Metalworks demo
rem
rem @(#)runnit.bat	1.5 99/07/12

if "%SWING_HOME%" == "" goto nohome
@echo on
cd classes
java -classpath ".;Metalworks.jar;%SWING_HOME%;%SWING_HOME%\swing.jar;%SWING_HOME%\windows.jar;%SWING_HOME%\motif.jar;%SWING_HOME%\multi.jar;%CLASSPATH%" Metalworks
cd ..
@echo off
goto done

:nohome
echo No SWING_HOME environment variable set.

:done

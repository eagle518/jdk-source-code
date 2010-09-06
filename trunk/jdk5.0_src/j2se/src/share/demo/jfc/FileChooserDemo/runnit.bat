@echo off
rem Run the FileChooserDemo demo
rem
rem 1.3 97/11/26

if "%SWING_HOME%" == "" goto nohome
@echo on
java -classpath ".;FileChooserDemo.jar;%SWING_HOME%;%SWING_HOME%\swing.jar;%SWING_HOME%\windows.jar;%SWING_HOME%\motif.jar;%SWING_HOME%\multi.jar;%CLASSPATH%" FileChooserDemo
@echo off
goto done

:nohome
echo No SWING_HOME environment variable set.

:done


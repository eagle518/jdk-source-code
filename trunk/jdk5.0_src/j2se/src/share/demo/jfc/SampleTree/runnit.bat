@echo off
rem Run the SampleTree demo
rem
rem 1.1 97/11/24

if "%SWING_HOME%" == "" goto nohome
@echo on
java -classpath ".;SampleTree.jar;%SWING_HOME%;%SWING_HOME%\swing.jar;%SWING_HOME%\windows.jar;%SWING_HOME%\motif.jar;%SWING_HOME%\multi.jar;%CLASSPATH%" SampleTree
@echo off
goto done

:nohome
echo No SWING_HOME environment variable set.

:done

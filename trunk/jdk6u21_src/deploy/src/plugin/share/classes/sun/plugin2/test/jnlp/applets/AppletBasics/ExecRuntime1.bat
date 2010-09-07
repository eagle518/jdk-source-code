set LIBXCB_ALLOW_SLOPPY_LOCK=1
set JPI_PLUGIN2_DEBUG=1
set JPI_PLUGIN2_VERBOSE=1
set JAVAWS_TRACE_NATIVE=1

set J2RE_HOME=C:\jre1.6.0_10
set JAVA=%J2RE_HOME%\bin\java
set JAVAW=%J2RE_HOME%\bin\javaw

%JAVA% -classpath .  ExecRuntime %JAVAW% -client -Xbootclasspath/a:%J2RE_HOME%\lib\javaws.jar;%J2RE_HOME%\lib\deploy.jar -classpath %J2RE_HOME%\lib\deploy.jar -Djnlpx.jvm=%JAVAW% -Djnlpx.splashport=38582 -Djnlpx.home=%J2RE_HOME%\bin -Djnlpx.remove=false -Djnlpx.offline=false -Djnlpx.relaunch=true -Djnlpx.heapsize=NULL,NULL -Djava.security.policy=file:%J2RE_HOME%\lib\security\javaws.policy -DtrustProxy=true -Xverify:remote com.sun.javaws.Main -secure JNLPLongProperties1.2.abs.sec.jnlp


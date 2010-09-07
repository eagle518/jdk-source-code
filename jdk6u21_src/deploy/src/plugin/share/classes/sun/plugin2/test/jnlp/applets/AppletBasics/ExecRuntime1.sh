#! /bin/sh

BASEDIR=$(dirname $0)

UNIX=1
uname | grep WIN && UNIX=0

if [ $UNIX -eq 0 ] ; then
    . $BASEDIR/setenv.test.windows-i586.sh
else
    . $BASEDIR/setenv.test.linux-i586.sh
fi

export LIBXCB_ALLOW_SLOPPY_LOCK=1
export JPI_PLUGIN2_DEBUG=1
export JPI_PLUGIN2_VERBOSE=1
export JAVAWS_TRACE_NATIVE=1

URI=$1

JAVA=$J2RE_HOME/bin/java
echo JAVA: $JAVA

cd codebasedir

$JAVA \
-cp . \
ExecRuntime \
$JAVA \
-DQueryResult.LimitationCount=40 \
"-DQueryResult.WarningMessage=The query exceeds the allowed number of search terms.NL Results matching the first 40 terms have been returned.NL This may include more or less results than what the full query would have returned." \
-DminimumJRE=1.4.2 \
-client \
-Xbootclasspath/a:$J2RE_HOME/lib/javaws.jar:$J2RE_HOME/lib/deploy.jar \
-classpath \
$J2RE_HOME/lib/deploy.jar \
"-Djnlpx.vmargs=-DQueryResult.LimitationCount=40 \"-DQueryResult.WarningMessage=The query exceeds the allowed number of search terms.NL Results matching the first 40 terms have been returned.NL This may include more or less results than what the full query would have returned.\" -DminimumJRE=1.4.2 -client" \
-Djnlpx.jvm=$JAVA \
-Djnlpx.splashport=38582 \
-Djnlpx.home=$J2RE_HOME/bin \
-Djnlpx.remove=false \
-Djnlpx.offline=false \
-Djnlpx.relaunch=true \
-Djnlpx.heapsize=NULL,NULL \
-Djava.security.policy=file:$J2RE_HOME/lib/security/javaws.policy \
-DtrustProxy=true \
-Xverify:remote \
com.sun.javaws.Main \
-secure \
../scodebasedir/JNLPLongProperties1.2.abs.sec.jnlp


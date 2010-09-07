#! /bin/sh

BASEDIR=$(dirname $0)

UNIX=1
uname | grep WIN && UNIX=0

if [ $UNIX -eq 0 ] ; then
    . $BASEDIR/setenv.test.windows-i586.sh
else
    . $BASEDIR/setenv.test.linux-i586.sh
fi

PATH=$JAVA_HOME/bin:$PATH

JAVAC=$JAVA_HOME/bin/javac

JAVAWS=$J2RE_HOME/lib/javaws.jar
PLUGIN=$J2RE_HOME/lib/plugin.jar
DEPLOY=$J2RE_HOME/lib/deploy.jar

CLASSPATH=$JAVAWS$CP_SEP$PLUGIN$CP_SEP$DEPLOY$CP_SEP$CLASSPATH
export CLASSPATH

echo $JAVAC $CLASSPATH
$JAVAC -source 1.4 -target 1.4 -classpath $CLASSPATH *.java

cd native
make

#! /bin/sh

BASEDIR=$(dirname $0)

. $BASEDIR/setenv.test.linux-i586.sh

DEPLOY=$JAVA_HOME/lib/deploy.jar
JAVAWS=$JAVA_HOME/lib/javaws.jar
PLUGIN=$JAVA_HOME/lib/plugin.jar

URI=$1
ok=0 ; echo $URI | grep "^http://" && ok=1
if [ $ok -eq 1 ] ; then
    echo using abs URI
fi
if [ $ok -eq 0 ] ; then
    URI="file://"$(pwd)"/$1"
    echo using rel URI
fi

java -Xbootclasspath/a:$DEPLOY:$JAVAWS:$PLUGIN \
    sun.plugin2.applet.viewer.Applet2Viewer $URI


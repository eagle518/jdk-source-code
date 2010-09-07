#! /bin/sh

BASEDIR=$(dirname $0)

. $BASEDIR/setenv.test.linux-i586.sh

DEPLOY=$J2RE_HOME/lib/deploy.jar

JAVAWS=$J2RE_HOME/lib/javaws.jar

PLUGIN=$J2RE_HOME/lib/plugin.jar

export JPI_PLUGIN2_DEBUG=1
export JPI_PLUGIN2_VERBOSE=1

export LIBXCB_ALLOW_SLOPPY_LOCK=1

URI=$1
ok=0 ; echo $URI | grep "^http://" && ok=1
if [ $ok -eq 1 ] ; then
    echo using abs URI
fi
if [ $ok -eq 0 ] ; then
    URI="file://"$(pwd)"/$1"
    echo using rel URI
fi
which java
$J2RE_HOME/bin/java -Xbootclasspath/a:$DEPLOY:$JAVAWS:$PLUGIN \
    sun.plugin2.applet.viewer.JNLP2Viewer $URI


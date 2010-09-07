#! /bin/sh

BASEDIR=$(dirname $0)

UNIX=1
uname | grep WIN && UNIX=0

if [ $UNIX -eq 0 ] ; then
    . $BASEDIR/setenv.test.windows-i586.sh
else
    . $BASEDIR/setenv.test.linux-i586.sh
fi

$J2RE_HOME/bin/java -Xbootclasspath/a:$J2RE_HOME/lib/deploy.jar server1RWThreads $*


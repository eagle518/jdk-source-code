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

echo $J2RE_HOME/bin/javaws

$J2RE_HOME/bin/javaws $URI


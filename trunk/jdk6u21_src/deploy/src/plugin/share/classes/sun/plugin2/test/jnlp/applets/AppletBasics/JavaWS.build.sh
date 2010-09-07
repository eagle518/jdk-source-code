#! /bin/sh

BASEDIR=$(dirname $0)

. $BASEDIR/setenv.test.build.sh $1
shift

export JPI_PLUGIN2_DEBUG=1
export JPI_PLUGIN2_VERBOSE=1

URI=$1

echo $J2RE_HOME/bin/javaws

$J2RE_HOME/bin/javaws $URI


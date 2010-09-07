#! /bin/sh

BASEDIR=$(dirname $0)

. $BASEDIR/setenv.test.linux-i586.sh

export JPI_PLUGIN2_DEBUG=1
export JPI_PLUGIN2_VERBOSE=1

/opt-linux-x86/MozillaFirefox3/firefox file://$(pwd)/$1

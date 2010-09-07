#! /bin/sh

BASEDIR=$(dirname $0)

. $BASEDIR/setenv.test.linux-i586.sh

export JPI_PLUGIN2_DEBUG=1
export JPI_PLUGIN2_VERBOSE=1
export JPI_PLUGIN2_FORCE_DRAGGABLE=1
export DEPLOY_PERF_ENABLED=1

export LIBXCB_ALLOW_SLOPPY_LOCK=1

URI=$1
shift
if [ ! -z "$URI" ] ; then
    ok=0 ; echo $URI | grep "^http://" && ok=1
    if [ $ok -eq 1 ] ; then
        echo using abs URI
    fi
    if [ $ok -eq 0 ] ; then
        URI="file://"$(pwd)"/$URI"
        echo using rel URI
    fi
fi


/opt-linux-x86/MozillaFirefox3/firefox $URI

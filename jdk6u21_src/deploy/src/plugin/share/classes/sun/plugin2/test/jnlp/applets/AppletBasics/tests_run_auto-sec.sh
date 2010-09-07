#! /bin/sh

LOGDIR=$1

if [ -z "$LOGDIR" ] ; then
    echo Usage $0 "<logdir>"
fi

. ./tests_run_auto-func.sh

#set -x 

rm -rf $LOGDIR
mkdir -p $LOGDIR

# dialog box ...
rm -f /tmp/jnlp-tmpfile.txt
touch /tmp/jnlp-tmpfile.txt
JNLPAppletTest1 JNLPExtendedService1-jnlp-applet-sec.html JNLPExtendedService1-jnlp-applet-sec.as-jnlp-applet-1
JNLPAppletTest2 JNLPExtendedService1-jnlp-applet-sec.html JNLPExtendedService1-jnlp-applet-sec.as-jnlp-applet-2


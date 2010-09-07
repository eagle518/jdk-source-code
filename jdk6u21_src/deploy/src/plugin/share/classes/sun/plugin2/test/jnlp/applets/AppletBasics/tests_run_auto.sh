#! /bin/sh

LOGDIR=$1

if [ -z "$LOGDIR" ] ; then
    echo Usage $0 "<logdir>"
fi

. ./tests_run_auto-func.sh

#set -x 

rm -rf $LOGDIR
mkdir -p $LOGDIR

JavaAppletTest SimpleApplet1-java-applet-rel.html SimpleApplet1-java-applet-rel.as-java-applet

JNLPAppletTest1 SimpleApplet1-jnlp-applet-rel.html SimpleApplet1-jnlp-applet-rel.as-jnlp-applet-1
JNLPAppletTest2 SimpleApplet1-jnlp-applet-rel.html SimpleApplet1-jnlp-applet-rel.as-jnlp-applet-2

JavaAppletTest SimpleApplet1-jnlp-applet-rel.html SimpleApplet1-jnlp-applet-rel.as-java-applet

JNLPAppletTest1 JNLPBasicService1-jnlp-applet-rel.html JNLPBasicService1-jnlp-applet-rel.as-jnlp-applet-1
JNLPAppletTest2 JNLPBasicService1-jnlp-applet-rel.html JNLPBasicService1-jnlp-applet-rel.as-jnlp-applet-2

JNLPAppletTest2xt1_1 JNLPSingleInstanceService1-jnlp-applet-rel.html JNLPSingleInstanceService1-jnlp-applet-rel.as-jnlp-applet-1
JNLPAppletTest2xt1_2 JNLPSingleInstanceService1-jnlp-applet-rel.html JNLPSingleInstanceService1-jnlp-applet-rel.as-jnlp-applet-2

# dialog box ...
#rm -f /tmp/jnlp-tmpfile.txt
#touch /tmp/jnlp-tmpfile.txt
#JNLPAppletTest1 JNLPExtendedService1-jnlp-applet-rel.html JNLPExtendedService1-jnlp-applet-rel.as-jnlp-applet-1
#JNLPAppletTest2 JNLPExtendedService1-jnlp-applet-rel.html JNLPExtendedService1-jnlp-applet-rel.as-jnlp-applet-2

JNLPAppletTest1 JNLPPersistenceService1-jnlp-applet-rel-round0.html JNLPPersistenceService1-jnlp-applet-rel.as-jnlp-applet-round0
JNLPAppletTest2 JNLPPersistenceService1-jnlp-applet-rel-round1.html JNLPPersistenceService1-jnlp-applet-rel.as-jnlp-applet-round1
JNLPAppletTest2 JNLPPersistenceService1-jnlp-applet-rel-round2.html JNLPPersistenceService1-jnlp-applet-rel.as-jnlp-applet-round2

JNLPAppletTest1 JRERelaunch1-jnlp-applet-rel-nop.html JRERelaunch1-jnlp-applet-rel-nop-1
JNLPAppletTest2 JRERelaunch1-jnlp-applet-rel-nop.html JRERelaunch1-jnlp-applet-rel-nop-2

JNLPAppletTest1 SimpleApplet1-jnlp-applet-cbt1.html SimpleApplet1-jnlp-applet-cbt1-1
JNLPAppletTest2 SimpleApplet1-jnlp-applet-cbt1.html SimpleApplet1-jnlp-applet-cbt1-2

JNLPAppletTest1 SimpleApplet1-jnlp-applet-cbt2.html SimpleApplet1-jnlp-applet-cbt2-1
JNLPAppletTest2 SimpleApplet1-jnlp-applet-cbt2.html SimpleApplet1-jnlp-applet-cbt2-2

JNLPAppletTest1 subdir/SimpleApplet1-jnlp-applet-cbt3p2.html SimpleApplet1-jnlp-applet-cbt3-1
JNLPAppletTest1 subdir/SimpleApplet1-jnlp-applet-cbt3p2.html SimpleApplet1-jnlp-applet-cbt3-2

JNLPAppletTest1 subdir/SimpleApplet1-jnlp-applet-cbt4p2.html SimpleApplet1-jnlp-applet-cbt4-1
JNLPAppletTest1 subdir/SimpleApplet1-jnlp-applet-cbt4p2.html SimpleApplet1-jnlp-applet-cbt4-2


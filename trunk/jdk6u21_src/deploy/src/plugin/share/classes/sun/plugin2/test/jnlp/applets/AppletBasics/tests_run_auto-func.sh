#! /bin/sh

#set -x 

export SLEEP_INIT=6s
export SLEEP_NEXT=4s

function JNLPAppletTest1()
{
    sh JavaWS.sh -uninstall
    echo JNLPAppletTest $1 $LOGDIR/$2.log
    sh JNLP2ViewerNoP2Jar.sh   $1 >& $LOGDIR/$2.log & 
    sleep $SLEEP_INIT
    killall java
    DONE=0 ; grep TestResult $LOGDIR/$2.log && DONE=1
    if [ $DONE -eq 0 ] ; then 
        echo TestResult failed - aborted
    fi
}

function JNLPAppletTest2()
{
    echo JNLPAppletTest $1 $LOGDIR/$2.log
    sh JNLP2ViewerNoP2Jar.sh   $1 >& $LOGDIR/$2.log & 
    sleep $SLEEP_NEXT
    killall java
    DONE=0 ; grep TestResult $LOGDIR/$2.log && DONE=1
    if [ $DONE -eq 0 ] ; then 
        echo TestResult failed - aborted
    fi
}

function JNLPAppletTest2xt1_1()
{
    sh JavaWS.sh -uninstall
    echo JNLPAppletTest2xt1 $1 $LOGDIR/$2-i1.log 
    sh JNLP2ViewerNoP2Jar.sh   $1 >& $LOGDIR/$2-i1.log & 
    sleep $SLEEP_INIT
    sh JNLP2ViewerNoP2Jar.sh   $1 >& $LOGDIR/$2-i2.log & 
    sleep $SLEEP_NEXT
    killall java
    DONE=0 ; grep TestResult $LOGDIR/$2-i1.log && DONE=1
    if [ $DONE -eq 0 ] ; then 
        echo TestResult failed - aborted
    fi
}

function JNLPAppletTest2xt1_2()
{
    echo JNLPAppletTest2xt1 $1 $LOGDIR/$2-i1.log 
    sh JNLP2ViewerNoP2Jar.sh   $1 >& $LOGDIR/$2-i1.log & 
    sleep $SLEEP_NEXT
    sh JNLP2ViewerNoP2Jar.sh   $1 >& $LOGDIR/$2-i2.log & 
    sleep $SLEEP_NEXT
    killall java
    DONE=0 ; grep TestResult $LOGDIR/$2-i1.log && DONE=1
    if [ $DONE -eq 0 ] ; then 
        echo TestResult failed - aborted
    fi
}

function JavaAppletTest()
{
    echo JavaAppletTest $1 $LOGDIR/$2.log
    sh Applet2ViewerNoP2Jar.sh $1 >& $LOGDIR/$2.log & 
    sleep $SLEEP_NEXT
    killall java
    DONE=0 ; grep TestResult $LOGDIR/$2.log && DONE=1
    if [ $DONE -eq 0 ] ; then 
        echo TestResult failed - aborted
    fi
}

function JAVAWSTest1()
{
    sh JavaWS.sh -uninstall
    echo
    mkdir -p $LOGDIR/$2
    echo JAVAWS Test $1 $LOGDIR/$2
    rm -f $JAVALOGDIR/*
    sh JavaWS.sh $1 
    sleep $SLEEP_INIT
    killall java
    cp $JAVALOGDIR/*.trace $LOGDIR/$2/
    DONE=0 ; grep -h TestResult $LOGDIR/$2/*.trace && DONE=1
    if [ $DONE -eq 0 ] ; then 
        echo TestResult failed - aborted
    fi
    echo 
}

function JAVAWSTest2()
{
    echo
    mkdir -p $LOGDIR/$2
    echo JAVAWS Test $1 $LOGDIR/$2
    rm -f $JAVALOGDIR/*
    sh JavaWS.sh $1 
    sleep $SLEEP_NEXT
    killall java
    cp $JAVALOGDIR/*.trace $LOGDIR/$2/
    DONE=0 ; grep -h TestResult $LOGDIR/$2/*.trace && DONE=1
    if [ $DONE -eq 0 ] ; then 
        echo TestResult failed - aborted
    fi
    echo 
}


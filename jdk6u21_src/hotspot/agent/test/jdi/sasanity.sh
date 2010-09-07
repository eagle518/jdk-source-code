#!/bin/ksh
#
# Copyright (c) 2003, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#  
#

# This script is used to run sanity check on vmStructs.
# Each SA class is checked against a given VM. "PASSED" is 
# printed if vmStructs are consistent. Else, "FAILED" is
# printed and an exception stack trace follows.

usage() {
    echo "usage: ./sasanity.sh <jdk>"
    echo "<jdk> is the 1.5 j2se directory against which you want to run sanity check"
    exit 1   
}

if [ "$1" == "" ]; then
    usage
fi

if [ "$1" == "-help" ]; then
    usage
fi

jdk=$1
OS=`uname`

if [ "$OS" != "Linux" ]; then
   OPTIONS="-Dsun.jvm.hotspot.debugger.useProcDebugger"
fi

javacp=$jdk/lib/sa-jdi.jar:./workdir

mkdir -p workdir
if [ SASanityChecker.java -nt ./workdir/SASanityChecker.class ] ; then
    $jdk/bin/javac -d ./workdir -classpath $javacp SASanityChecker.java
    if [ $? != 0 ] ; then
        exit 1
    fi
fi

if [ sagtarg.java -nt ./workdir/sagtarg.class ]; then
    $jdk/bin/javac -g  -classpath -d $workdir sagtarg.java
    if [ $? != 0 ] ; then
        exit 1
    fi
fi

tmp=/tmp/sagsetup
rm -f $tmp
$jdk/bin/java sagtarg > $tmp &
pid=$!
while [ ! -s $tmp ] ; do
  # Kludge alert!
  sleep 2
done

$jdk/bin/java -showversion ${OPTIONS} -classpath $javacp SASanityChecker $pid
kill -9 $pid

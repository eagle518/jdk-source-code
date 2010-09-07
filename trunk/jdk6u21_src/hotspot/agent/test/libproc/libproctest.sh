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

# This script is used to run consistency check of Serviceabilty Agent
# after making any libproc.so changes. Prints "PASSED" or "FAILED" in
# standard output.

usage() {
    echo "usage: $0"
    echo "   set SA_JAVA to be java executable from JDK 1.5"
    exit 1 
}

STARTDIR=`dirname $0`

if [ "$1" == "-help" ]; then
    usage
fi

if [ "x$SA_JAVA" = "x" ]; then
   SA_JAVA=java
fi

# create java process with test case
tmp=/tmp/libproctest
rm -f $tmp
$SA_JAVA -classpath $STARTDIR LibprocTest > $tmp &
pid=$!
while [ ! -s $tmp ] ; do
  # Kludge alert!
  sleep 2
done

# dump core
gcore $pid
kill -9 $pid


OPTIONS="-Djava.library.path=$STARTDIR/../src/os/solaris/proc/`uname -p`:$STARTDIR/../solaris/`uname -p` -Dsun.jvm.hotspot.debugger.useProcDebugger"

# run libproc client
$SA_JAVA -showversion ${OPTIONS} -cp $STARTDIR/../../build/classes::$STARTDIR/../sa.jar:$STARTDIR LibprocClient x core.$pid

# delete core
rm -f core.$pid

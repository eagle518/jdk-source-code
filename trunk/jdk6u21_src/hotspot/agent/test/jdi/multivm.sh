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

doUsage()
{
    cat <<EOF
    Run multivm.class using Serviceability Agent to talk to 2 pids
    simultaneousely. i.e, before detaching one attach another.
    Usage:  multivm.sh <jdk-pathname> <pid1> <pid2>

EOF
}

if [ $# = 4 ] ; then
    doUsage
    exit 1
fi

jdk=$1
javacp="$jdk/lib/sa-jdi.jar:$classesDir:$jdk/lib/tools.jar:$jdk/classes:./workdir"

mkdir -p workdir
if [ sagdoit.java -nt ./workdir/sagdoit.class ] ; then
    $jdk/bin/javac -d ./workdir -classpath $javacp sagdoit.java
    if [ $? != 0 ] ; then
        exit 1
    fi
fi
if [ multivm.java -nt ./workdir/multivm.class ] ; then
    $jdk/bin/javac -d ./workdir -classpath $javacp multivm.java
    if [ $? != 0 ] ; then
        exit 1
    fi
fi

$jdk/bin/java -Dsun.jvm.hotspot.jdi.ConnectorImpl.DEBUG -Dsun.jvm.hotspot.jdi.SAJDIClassLoader.DEBUG -Djava.class.path=$javacp multivm $2 $3

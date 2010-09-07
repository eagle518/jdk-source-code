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
    Run serialvm.class using Serviceability Agent to talk to 2 pids one
    after another in serial fashion.
    Usage:  serialvm.sh <jdk-pathname> <pid1> <pid2>

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
if [ serialvm.java -nt ./workdir/serialvm.class ] ; then
    $jdk/bin/javac -d ./workdir -classpath $javacp serialvm.java
    if [ $? != 0 ] ; then
        exit 1
    fi
fi

$jdk/bin/java -Dsun.jvm.hotspot.jdi.ConnectorImpl.DEBUG -Dsun.jvm.hotspot.jdi.SAJDIClassLoader.DEBUG -Djava.class.path=$javacp serialvm $2 $3

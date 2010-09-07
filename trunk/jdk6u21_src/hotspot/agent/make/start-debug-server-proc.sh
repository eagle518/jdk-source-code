#!/bin/sh
#
# Copyright (c) 2002, 2003, Oracle and/or its affiliates. All rights reserved.
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

. `dirname $0`/saenv.sh

if [ -f $STARTDIR/sa.jar ] ; then
  CP=$STARTDIR/sa.jar
else
  CP=$STARTDIR/../build/classes
fi

$SA_JAVA -classpath $CP ${OPTIONS} -Djava.rmi.server.codebase=file:/$CP -Djava.security.policy=$STARTDIR\/grantAll.policy sun.jvm.hotspot.DebugServer $*

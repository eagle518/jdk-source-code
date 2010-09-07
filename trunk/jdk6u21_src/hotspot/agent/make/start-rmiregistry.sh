#!/bin/sh
#
# Copyright (c) 2000, 2003, Oracle and/or its affiliates. All rights reserved.
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

STARTDIR=`dirname $0`

if [ -f $STARTDIR/sa.jar ] ; then
  CP=$STARTDIR/sa.jar
else
  CP=$STARTDIR/../build/classes
fi

rmiregistry -J-Xbootclasspath/p:$CP

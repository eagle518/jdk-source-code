#!/bin/sh
#
# Copyright (c) 2002, 2008, Oracle and/or its affiliates. All rights reserved.
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

if [ "x$SA_JAVA" = "x" ]; then
   SA_JAVA=java
fi

$SA_JAVA -showversion -cp $STARTDIR/../build/classes:$STARTDIR/../src/share/lib/js.jar:$STARTDIR/sa.jar:$STARTDIR/lib/js.jar sun.jvm.hotspot.HSDB $*

#!/bin/sh
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

OS=`uname`

if [ "$OS" != "Linux" ]; then
   OPTIONS="-Dsun.jvm.hotspot.debugger.useProcDebugger"
fi

$JAVA_HOME/bin/java -showversion ${OPTIONS} -classpath $JAVA_HOME/lib/sa-jdi.jar sun.jvm.hotspot.tools.StackTrace $*

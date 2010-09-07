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

OPTIONS="-Dsun.jvm.hotspot.debugger.useProcDebugger"

$JAVA_HOME/bin/java -d64 -showversion ${OPTIONS} -classpath $JAVA_HOME/lib/sa-jdi.jar sun.jvm.hotspot.tools.StackTrace $*

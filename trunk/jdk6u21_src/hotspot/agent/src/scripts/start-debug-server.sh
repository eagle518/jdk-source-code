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

if [ "$1" = "-help" ] ; then
     echo "Usage: $0 <pid>"
     echo "       $0 <java executable> <core file>"
     echo " Start the JDI debug server on <pid> or <core file>"
     echo " so that it can be debugged from a remote machine."
     echo " JAVA_HOME must contain the pathname of a J2SE 1.5"
     echo " installation."
     exit 0
fi

if [ ! -x ${JAVA_HOME}/bin/java -o ! -r ${JAVA_HOME}/lib/sa-jdi.jar ] ; 
then
    echo '${JAVA_HOME} does not point to a working J2SE 1.5 installation.'
    exit 1
fi

${JAVA_HOME}/bin/java -classpath ${JAVA_HOME}/lib/sa-jdi.jar sun.jvm.hotspot.jdi.SADebugServer $*

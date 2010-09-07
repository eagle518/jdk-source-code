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
    echo "usage: $0&"
    echo " Start the rmi registry with with sa-jdi.jar on the bootclasspath"
    echo " for use by the debug server."
    echo " JAVA_HOME must contain the pathname of a J2SE 1.5" 
    echo " installation." 
    exit 0
fi

if [ ! -x ${JAVA_HOME}/bin/rmiregistry -o ! -r ${JAVA_HOME}/lib/sa-jdi.jar ] ; 
then
    echo '${JAVA_HOME} does not point to a working J2SE installation.'
    exit 1
fi

${JAVA_HOME}/bin/rmiregistry -J-d64 -J-Xbootclasspath/p:${JAVA_HOME}/lib/sa-jdi.jar

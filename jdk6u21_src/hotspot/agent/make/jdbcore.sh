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

usage()
{
  echo "usage:   $0 <java executable> <core file>"
  exit 1
}
#
if [ $# -lt 2 ]; then
    usage
else
    EXEC_FILE="${1}"
    CORE_FILE="${2}"
    echo "$0 attaching to core=${CORE_FILE}"
fi
#

. `dirname $0`/saenv.sh

$JAVA_HOME/bin/jdb -J-Xbootclasspath/a:$SA_CLASSPATH:$JAVA_HOME/lib/tools.jar \
 -J-Dsun.boot.library.path=$JAVA_HOME/jre/lib/$CPU:$SA_LIBPATH \
 -connect sun.jvm.hotspot.jdi.SACoreAttachingConnector:core=${CORE_FILE},javaExecutable=${EXEC_FILE}

#!/bin/sh
#
# Copyright (c) 2003, 2009, Oracle and/or its affiliates. All rights reserved.
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

# This file sets common environment variables for all 64-bit Solaris [sparcv9,
# amd64] SA scripts. Please note that for 64-bit Linux use saenv.sh.

OS=`uname`
STARTDIR=`dirname $0`

CPU=`isainfo | grep sparcv9`

if [ "x$CPU" != "x" ]; then
  CPU=sparcv9
else 
  CPU=`isainfo | grep amd64`
  if [ "x$CPU" != "x" ]; then
     CPU=amd64
  else
     echo "unknown CPU, only sparcv9, amd64 are supported!"
     exit 1
  fi
fi

# configure audit helper library if SA_ALTROOT is set
if [ -n "$SA_ALTROOT" ]; then
  LD_AUDIT_64=$STARTDIR/../src/os/solaris/proc/$CPU/libsaproc_audit.so
  export LD_AUDIT_64
  if [ ! -f $LD_AUDIT_64 ]; then
      echo "SA_ALTROOT is set and can't find libsaproc_audit.so."
      echo "Make sure to build it with 'make natives'."
      exit 1
  fi
fi
SA_LIBPATH=$STARTDIR/../src/os/solaris/proc/$CPU:$STARTDIR/solaris/$CPU

OPTIONS="-Dsa.library.path=$SA_LIBPATH -Dsun.jvm.hotspot.debugger.useProcDebugger"

if [ "x$SA_JAVA" = "x" ]; then
   SA_JAVA=java
fi

if [ "x$SA_DISABLE_VERS_CHK" != "x" ]; then
   OPTIONS="-Dsun.jvm.hotspot.runtime.VM.disableVersionCheck ${OPTIONS}"
fi

SA_CLASSPATH=$STARTDIR/../build/classes:$STARTDIR/../src/share/lib/js.jar:$STARTDIR/sa.jar::$STARTDIR/lib/js.jar

OPTIONS="-Djava.system.class.loader=sun.jvm.hotspot.SALauncherLoader ${OPTIONS}"

SA_JAVA_CMD="$SA_PREFIX_CMD $SA_JAVA -d64 -showversion ${OPTIONS} -cp $SA_CLASSPATH $SA_OPTIONS"

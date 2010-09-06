#! /bin/sh
# 
# @(#)build.sh	1.5 03/12/23 16:35:14
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# Make sure the variable JAVA_HOME is set before running this script.

set -u


if [ $# != 2 ]; then 
    echo "Usage : $0 Build_Options Location"
    echo "Build Options : debug or optimized or basicdebug or basic or clean"
    echo "Location : specify any workspace which has gamma sources"
    exit 1
fi

# Just in case:
case ${JAVA_HOME} in
/*) true;;
?*) JAVA_HOME=`( cd $JAVA_HOME; pwd )`;;
esac

case `uname -m` in
  i386|i486|i586|i686)
    mach=i386
    ;;
  *)
    echo "Unsupported machine: " `uname -m`
    exit 1
    ;;
esac

if [ "${JAVA_HOME}" = ""  -o  ! -d "${JAVA_HOME}" -o ! -d ${JAVA_HOME}/jre/lib/${mach} ]; then
    echo "JAVA_HOME needs to be set to a valid JDK path"
    echo "ksh : export JAVA_HOME=/net/tetrasparc/export/gobi/JDK1.2_fcs_V/linux"
    echo "csh : setenv JAVA_HOME /net/tetrasparc/export/gobi/JDK1.2_fcs_V/linux"
    exit 1
fi


LD_LIBRARY_PATH=${JAVA_HOME}/jre/lib/`uname -p`:\
${JAVA_HOME}/jre/lib/`uname -p`/native_threads:${LD_LIBRARY_PATH-.}

# This is necessary as long as we are using the old launcher
# with the new distribution format:
CLASSPATH=${JAVA_HOME}/jre/lib/rt.jar:${CLASSPATH-.}


for gm in gmake gnumake
do
  if [ "${GNUMAKE-}" != "" ]; then break; fi
  ($gm --version >/dev/null) 2>/dev/null && GNUMAKE=$gm
done
: ${GNUMAKE:?'Cannot locate the gnumake program.  Stop.'}


echo "### ENVIRONMENT SETTINGS:"
export JAVA_HOME		; echo "JAVA_HOME=$JAVA_HOME"
export LD_LIBRARY_PATH		; echo "LD_LIBRARY_PATH=$LD_LIBRARY_PATH"
export CLASSPATH		; echo "CLASSPATH=$CLASSPATH"
export GNUMAKE			; echo "GNUMAKE=$GNUMAKE"
echo "###"

Build_Options=$1
Location=$2

case ${Location} in
/*) true;;
?*) Location=`(cd ${Location}; pwd)`;;
esac

echo \
${GNUMAKE} -f ${Location}/build/linux/Makefile $Build_Options GAMMADIR=${Location}
${GNUMAKE} -f ${Location}/build/linux/Makefile $Build_Options GAMMADIR=${Location}

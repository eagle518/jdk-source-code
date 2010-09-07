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

. `dirname $0`/saenv.sh

type c++filt 1>/dev/null 2>/dev/null
if [ $? -eq 0 ]; then
   $SA_JAVA_CMD sun.jvm.hotspot.tools.PStack $* | c++filt
else
   $SA_JAVA_CMD sun.jvm.hotspot.tools.PStack $*
fi


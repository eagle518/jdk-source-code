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

$SA_JAVA_CMD sun.jvm.hotspot.tools.HeapSummary $*

#
# @(#)Makefile	1.2 10/03/24
#
# Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

JAVA=java
CLASSPATH_SEPARATOR=\;

default all::
	$(JAVA) -cp gluegen.jar$(CLASSPATH_SEPARATOR)antlr.jar com.sun.gluegen.GlueGen -Cwindows.cfg stub_includes/windows/windows.h

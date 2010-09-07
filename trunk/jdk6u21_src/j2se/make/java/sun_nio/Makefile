#
# @(#)Makefile	1.9 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for sun/nio files
#

BUILDDIR = ../..
PACKAGE = sun.nio
PRODUCT = sun

include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile.
#
include FILES_java.gmk

#
# Install .lib file.
#
INSTALL_DOT_LIB = true

#
# Bootstrap stage.
#
JAVAC_CMD = $(JAVAC_BOOT_CMD) 

include $(BUILDDIR)/common/Classes.gmk

clean:: 
	$(RM) -r $(CLASSHDRDIR)
clobber:: 
	$(RM) -r $(CLASSBINDIR)/sun/nio/cs 


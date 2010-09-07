#
# @(#)Makefile	1.18 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ..
PACKAGE = sunw
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
FILES_java = \
	sunw/util/EventObject.java \
        sunw/util/EventListener.java \
        sunw/io/Serializable.java

#
# include rules
#
include $(BUILDDIR)/common/Rules.gmk

#
# Rules
#
clean clobber:: .delete.classlist
	$(RM) -r $(CLASSBINDIR)/sunw


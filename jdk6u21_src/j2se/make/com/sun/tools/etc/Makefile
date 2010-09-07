#
# @(#)Makefile	1.2 05/11/17 14:12:25
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#


BUILDDIR = ../../../..
PACKAGE = com.sun.tools.etc
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

AUTO_FILES_JAVA_DIRS = com/sun/tools/etc

#
# Files that need to be copied
#
SERVICEDIR = $(CLASSBINDIR)/META-INF/services
                                                                                                
FILES_copy = \
	$(SERVICEDIR)/com.sun.mirror.apt.AnnotationProcessorFactory \
	$(SERVICEDIR)/com.sun.tools.internal.xjc.Plugin

include $(BUILDDIR)/common/Classes.gmk

all: classes copy-files

#
# Copy the service provider configuration file into the resource
# directory.
#

copy-files: $(FILES_copy)
                                                                                                
$(SERVICEDIR)/%: $(SHARE_SRC)/classes/com/sun/tools/etc/META-INF/services/%
	$(install-file)
                                                                                                
.PHONY: copy-files


clean::
	$(RM) -r $(CLASSDESTDIR)/com/sun/tools/etc
	$(RM) $(FILES_copy)


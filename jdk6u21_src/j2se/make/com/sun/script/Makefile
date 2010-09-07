#
# @(#)Makefile	1.3 10/03/23 16:59:29
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#


BUILDDIR = ../../..
PACKAGE = com.sun.script
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

AUTO_FILES_JAVA_DIRS = com/sun/script

#
# Files that need to be copied
#
SERVICEDIR = $(CLASSBINDIR)/META-INF/services
                                                                                                
FILES_copy = \
	$(SERVICEDIR)/javax.script.ScriptEngineFactory

include $(BUILDDIR)/common/Classes.gmk

all: classes copy-files

#
# Copy the service provider configuration file into the resource
# directory.
#

copy-files: $(FILES_copy)
                                                                                                
$(SERVICEDIR)/%: $(SHARE_SRC)/classes/com/sun/script/javascript/META-INF/services/%
	$(install-file)
                                                                                                
.PHONY: copy-files


clean::
	$(RM) -r $(CLASSDESTDIR)/com/sun/script/javascript
	$(RM) $(FILES_copy)


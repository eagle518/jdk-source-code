#
# @(#)Makefile	1.3 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#


BUILDDIR = ../../../..
PACKAGE = com.sun.tools.attach
LIBRARY = attach
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

ifeq ($(PLATFORM), solaris)
FILES_m = mapfile-solaris
endif
ifeq ($(PLATFORM), linux)
FILES_m = mapfile-linux
endif
include $(BUILDDIR)/common/Mapfile-vers.gmk

#
# Files to compile
#
include FILES_c.gmk
include FILES_java.gmk
include Exportedfiles.gmk

ifeq ($(PLATFORM), solaris)
OTHER_LDLIBS += -ldoor
endif

vpath %.c $(PLATFORM_SRC)/native/sun/tools/attach

all: classes copy-files

#
# Copy the service provider configuration files into the resource
# directory. Copy the configuration file also uncomments any
# providers that need to be enabled for this platform.
#
                                                                                                
copy-files: $(FILES_copy)
                                                                                                
$(SERVICEDIR)/%: $(SHARE_SRC)/classes/sun/tools/attach/META-INF/services/%
	@$(MKDIR) -p $(@D)
	@$(RM) $@
	@$(CAT) $< | $(SED) -e "s/^#\[$(PLATFORM)\]//" > $@
                                                                                                
.PHONY: copy-files


clean::
	$(RM) -r $(CLASSDESTDIR)/com/sun/tools/attach
	$(RM) -r $(CLASSDESTDIR)/sun/tools/attach
	$(RM) $(FILES_copy)

#
# Library to compile.
#
include $(BUILDDIR)/common/Library.gmk







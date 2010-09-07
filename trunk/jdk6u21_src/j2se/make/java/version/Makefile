#
# @(#)Makefile	1.15 10/03/23
#
# Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile to generate sun/misc/Version.java
#

BUILDDIR = ../..
include $(BUILDDIR)/common/Defs.gmk

all build: $(GENSRCDIR)/sun/misc/Version.java

ifeq ($(J4B), true)
  RUNTIME_NAME = Java(TM) Platform, Standard Edition for Business
else
  RUNTIME_NAME = Java(TM) SE Runtime Environment
endif

$(GENSRCDIR)/sun/misc/Version.java: \
		$(SHARE_SRC)/classes/sun/misc/Version-template.java
	$(prep-target)
	$(RM) $@.temp
	$(SED) -e 's/@@java_version@@/$(RELEASE)/g' \
	    -e 's/@@java_runtime_version@@/$(FULL_VERSION)/g' \
	    -e 's/@@java_runtime_name@@/$(RUNTIME_NAME)/g' \
	     $< > $@.temp
	@$(MV) $@.temp $@

clean clobber:
	$(RM) $(GENSRCDIR)/sun/misc/Version.java

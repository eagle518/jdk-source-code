#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
#  @(#)Makefile	1.29 10/03/23
#
#
# Most frequently used targets:
#
#    all            -- build optimized and debug versions of the core JDK and 
#                      ext (Defined in this makefile only)
#    clean          -- clobber the builds of the core JDK and ext (defined
#                      in this makefile only)
#
INSTALL_BUILDDIR=../../../..
PRODUCT = installer-windows

include $(INSTALL_BUILDDIR)/common/Defs.gmk

ifdef FASTJRE
  ifeq ($(ARCH_DATA_MODEL), 32)
    SUBDIRS = zipper jre wrapper
  endif
else
    SUBDIRS = zipper launcher touchfile jre sdk wrapper
endif

#This target will rebuild all of the JRE msi files and the full jre wrapper
#For the JDK, it will only build the plus target which just links in the new au2.0 msi
enhanced_plus:
	$(CD) zipper; $(MAKE) all
	$(CD) jre; $(MAKE) ENHANCED=true all
	$(CD) wrapper/wrapper.jre/; $(MAKE) all	
	$(CD) wrapper/wrapper.jdk/; $(MAKE) plus

all: sanity-bundles post-sanity-bundles
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE)  \
            || exit 1; $(CD) ..; \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done


clean::
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "clean" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) clean RELEASE="$(RELEASE)" FULL_VERSION="$(FULL_VERSION)" \
            || exit 1; $(CD) ..; \
	    $(ECHO) "<<<Finished Recursively making "$$i" all @ `$(DATE)`." ; \
	done

include $(INSTALL_BUILDDIR)/common/Sanity.gmk

#
# Phonies to avoid accidents.
#
.PHONY: all clean

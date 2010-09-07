#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
#  @(#)Makefile	1.18 06/04/04
#
#
# Most frequently used targets:
#
#    all            -- build optimized and debug versions of the core JDK and 
#                      ext (Defined in this makefile only)
#    clean          -- clobber the builds of the core JDK and ext (defined
#                      in this makefile only)
#
INSTALL_BUILDDIR=../../../../..
PRODUCT = installer-windows

include $(INSTALL_BUILDDIR)/common/Defs.gmk

ifdef FASTJRE
    SUBDIRS = wrapper.jre
else
  ifdef ALT_JRE_64BIT_FILEPATH
    ifneq ($(ARCH), amd64)
      SUBDIRS = wrapper.jre wrapper.jdk wrapper.jreboth
    else
      SUBDIRS = wrapper.jre wrapper.jdk
    endif
  else
    SUBDIRS = wrapper.jre wrapper.jdk
    COMBOWARNING64BIT = "Warning: The combo jre installer is not built since the 64-bit Installer is not defined"
  endif
endif

all: combo-warning build

build: sanity-bundles post-sanity-bundles
	@for i in $(SUBDIRS) ; do \
	    if [ $$i == "wrapper.jreboth" ] && [ ! -f $(ALT_JRE_64BIT_FILEPATH) ] ; then \
	    $(ECHO) "Warning: The combo jre installer is not built since the 64-bit Installer path is not found"; \
	    elif [ $$i == "wrapper.jreboth" ] && [ ! -f $(JRE_32BIT_FILEPATH) ] ; then \
	    $(ECHO) "Warning: The combo jre installer is not built since the 32-bit Installer path is not found"; \
	    else  \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE)  \
            || exit 1; $(CD) ..; \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	    fi  \
	done

combo-warning:
  ifdef COMBOWARNING64BIT
	@$(ECHO) $(COMBOWARNING64BIT)
  endif


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

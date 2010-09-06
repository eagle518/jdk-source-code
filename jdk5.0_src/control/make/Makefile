#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.264 04/01/29
#

BUILDDIR=.
include $(BUILDDIR)/common/Platform.gmk
include $(BUILDDIR)/common/Defs.gmk

all::
	@$(ECHO) $(PLATFORM) $(ARCH) $(RELEASE) build started: $(shell $(DATE) '+%y-%m-%d %H:%M')

include $(BUILDDIR)/sanity-rules.gmk
include $(BUILDDIR)/control-rules.gmk
include $(BUILDDIR)/hotspot-rules.gmk
include $(BUILDDIR)/motif-rules.gmk
include $(BUILDDIR)/j2se-rules.gmk
include $(BUILDDIR)/install-rules.gmk
include $(BUILDDIR)/deploy-rules.gmk


all:: sanity 

clobber::

ifeq ($(BUILD_HOTSPOT), true)
  all:: $(HOTSPOT) 
  clobber:: hotspot-clobber
endif

ifeq ($(BUILD_MOTIF), true)
  all:: $(MOTIF)
  clobber:: motif-clobber
endif

all:: $(J2SE) $(DEPLOY)

ifeq ($(BUILD_INSTALL), true)
  all :: $(INSTALL)
endif

ifndef DEV_ONLY
  ifndef SKIP_COMPARE_IMAGES
    all :: compare-images
  endif
endif

clobber:: j2se-clobber

ifeq ($(BUILD_INSTALL), true)
    clobber:: install-clobber
endif

clobber:: deploy-clobber clean-check
	$(RM) -r $(OUTPUTDIR)/*
	-($(RMDIR) -p $(OUTPUTDIR) > $(DEV_NULL) 2>&1; $(TRUE))

bringover: control-bringover hotspot-bringover motif-bringover j2se-bringover install-bringover deploy-bringover

source: j2se-source deploy-source

all:: 
	@$(ECHO) Control workspace build finished at `$(DATE)`


################################################################
#  PHONY
################################################################

.PHONY: all what clobber insane bringover \
	j2se-source deploy-source

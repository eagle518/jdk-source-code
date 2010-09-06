#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
#  @(#)Makefile	1.3 03/12/19
#
#
# Makefile for Rebasing JRE and SDK jar files prior to building bundles
#

INSTALL_BUILDDIR=..
include $(INSTALL_BUILDDIR)/common/Defs.gmk

all:: 
	@$(ECHO) $(ARCH) "Rebase  started:  " $(FULL_VERSION)

all optimized debug:: rebase-dlls 


rebase-dlls:: $(TMPDIR)
	$(FIND) $(JRE_IMAGE_DIR) -type f -name '?(*.$(LIBRARY_SUFFIX)|*.ocx)' -print | \
		$(GREP) -v $(DBG_SUFFIX) | \
	$(GREP) -v msvcrt.dll > $(TMPDIR)/jre-rebase.input
	$(REBASE) -b $(REBASE_ADDR) -R $(JRE_IMAGE_DIR) -G $(TMPDIR)/jre-rebase.input
 	
	
	$(FIND) $(JDK_IMAGE_DIR) -type f -name '?(*.$(LIBRARY_SUFFIX)|*.ocx)' -print | \
		$(GREP) -v $(DBG_SUFFIX) | \
	$(GREP) -v msvcrt.dll > $(TMPDIR)/jdk-rebase.input
	$(REBASE) -b $(REBASE_ADDR) -R $(JDK_IMAGE_DIR) -G $(TMPDIR)/jdk-rebase.input

clean:

clobber:


# this should be the last rule in this file:
all::
	@$(ECHO) $(ARCH) "Rebase finished:  " $(FULL_VERSION)
#
# Phonies to avoid accidents.
#
.PHONY: all rebase-dlls clean clobber

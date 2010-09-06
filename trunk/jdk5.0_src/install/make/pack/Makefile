#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
# @(#)Makefile	1.9 04/01/29
#
#
# Makefile for PACKing JRE and SDK jar files prior to building bundles
#

INSTALL_BUILDDIR=..
include $(INSTALL_BUILDDIR)/common/Defs.gmk
include $(INSTALL_BUILDDIR)/common/Pack.gmk


all:: 
	@$(ECHO) $(ARCH) "Installer Build started:  " $(FULL_VERSION)

all optimized debug:: pack-jre pack-sdk

pack-jre:
	$(MAKE) PACK_OUTPUTDIR=$(PACKED_JRE_JARDIR) \
		VAR_IMAGE_DIR=$(JRE_IMAGE_DIR) \
		JARS_TO_PACK="$(JRE_PACKED_JARS)" \
		EXTRA_PACKOPTS="--strip-debug" \
		pack-jars

pack-sdk:
	$(MAKE) PACK_OUTPUTDIR=$(PACKED_JDK_JARDIR) \
		VAR_IMAGE_DIR=$(JDK_IMAGE_DIR) \
		JARS_TO_PACK="$(JDK_PACKED_JARS)" \
		pack-jars

pack-jars:
	$(MKDIR) -p $(PACK_OUTPUTDIR)
	for i in $(JARS_TO_PACK) ; do \
	    $(LS) -l $(VAR_IMAGE_DIR)/$$i | $(NAWK) '{printf("Packing %s %s bytes\n",$$9, $$5)}'  ; \
	    $(MKDIR) -p $(PACK_OUTPUTDIR)/`$(DIRNAME) $$i` ; \
            $(MKDIR) -p $(VAR_IMAGE_DIR)/`$(DIRNAME) $$i` ; \
	    $(PACK_CMD) $(EXTRA_PACKOPTS) $(PACK_OUTPUTDIR)/`$(DIRNAME) $$i`/`$(BASENAME) $$i .jar`$(PACK_SUFFIX) $(VAR_IMAGE_DIR)/$$i || exit 1 ; \
	    $(RM) $(VAR_IMAGE_DIR)/$$i ; \
	    $(UNPACK_CMD) $(EXTRA_UNPACKOPTS) $(PACK_OUTPUTDIR)/`$(DIRNAME) $$i`/`$(BASENAME) $$i .jar`$(PACK_SUFFIX) $(VAR_IMAGE_DIR)/$$i || exit 1 ; \
	done

clean: clobber

clobber:
	rm -rf $(PACKED_JRE_JARDIR)
	rm -rf $(PACKED_JDK_JARDIR)


# this should be the last rule in this file:
all::
	@$(ECHO) $(ARCH) "Installer Build finished:  " $(FULL_VERSION)
#
# Phonies to avoid accidents.
#
.PHONY: all pack-jre pack-sdk pack-jars

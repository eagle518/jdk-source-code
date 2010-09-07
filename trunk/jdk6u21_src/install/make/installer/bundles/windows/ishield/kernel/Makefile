#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
# @(#)Makefile	1.8 10/03/23
#
# Finishes creating the j2re-image-kernel image
#

INSTALL_BUILDDIR= ../../../../..

include $(INSTALL_BUILDDIR)/common/Defs.gmk

KERNEL_TMPDIR    = $(OUTPUTDIR)/tmp/kernel
KERNEL_COPYDIR   = $(KERNEL_TMPDIR)/j2re-image-copy
JREIMAGE         = ${OUTPUTDIR}/tmp/ishield/patch/jre-image

ifdef EXTRA_COMP_LIB_PATH
  EXTRA_COMP_PROP_FLAG = \
    -Dkernel.extra.compression=true

  EXTRA_COMP_JAR_FLAG = \
    -C $(KERNEL_TMPDIR) $(EXTRA_COMP_LIB_NAME)
else
  EXTRA_COMP_PROP_FLAG = \
    -Dkernel.extra.compression=false
endif

all: kernel-image

kernel-image:
ifneq ($(ARCH), amd64)
	$(CAT) $(INSTALL_BUILDDIR)/pack/pack.all.properties \
	  pack.extra.properties \
	  > $(KERNEL_TMPDIR)/pack.properties
	$(JAVA) $(EXTRA_COMP_PROP_FLAG) \
	  -Dkernel.download.enabled=false -mx128m \
	  -Dkernel.deploy.debug=true -jar \
	  $(KERNEL_TMPDIR)/SplitJRE.jar $(OUTPUTDIR) \
	  $(KERNEL_TMPDIR)/pack.properties
	$(CD) $(KERNEL_TMPDIR); \
	  $(ZIPEXE) -r kernel_bundles bundles; \
	  $(CP) kernel_bundles.zip $(OUTPUTDIR)/bundles
endif

clean clobber:
	$(RM) -r $(OUTPUTDIR}/j2re-image-kernel

.PHONY: all clean clobber kernel-image

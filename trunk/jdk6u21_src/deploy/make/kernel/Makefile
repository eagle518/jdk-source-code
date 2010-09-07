#
# @(#)Makefile	1.23 10/03/24
#
# Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# Makefile to build the Java Kernel.
#
DEPLOY_TOPDIR = ../..

include ../common/Defs.gmk
include ../common/Sanity.gmk
include ../common/Rules.gmk
include ../common/Release.gmk

KERNEL_SRCDIR    = $(DEPLOY_TOPDIR)/src/kernel/share/classes
KERNEL_TMPDIR    = $(OUTPUTDIR)/tmp/kernel
KERNEL_COPYDIR   = $(KERNEL_TMPDIR)/j2re-image-copy
KERNEL_CLASSDIR  = $(KERNEL_TMPDIR)/classes

ifdef EXTRA_COMP_LIB_PATH
  EXTRA_COMP_PROP_FLAG = \
    -Dkernel.extra.compression=true

  EXTRA_COMP_JAR_FLAG = \
    -C $(KERNEL_TMPDIR) $(EXTRA_COMP_LIB_NAME)
else
  EXTRA_COMP_PROP_FLAG = \
    -Dkernel.extra.compression=false
endif

all: ${KERNEL_TMPDIR}/SplitJRE.jar

$(KERNEL_TMPDIR):
	${MKDIR} $@

$(KERNEL_CLASSDIR):
	${MKDIR} $@

$(KERNEL_CLASSDIR)/sun/jkernel:
	${MKDIR} $@

classes: $(KERNEL_TMPDIR) $(KERNEL_CLASSDIR) $(KERNEL_CLASSDIR)/sun/kernel/Bundle.class \
         $(KERNEL_CLASSDIR)/sun/kernel/SplitJRE.class \
         $(KERNEL_CLASSDIR)/sun/kernel/DownloadTest.class \
         $(KERNEL_CLASSDIR)/sun/kernel/SwingTest.class \
         $(KERNEL_CLASSDIR)/sun/kernel/SecurityTest.class \
         $(KERNEL_CLASSDIR)/sun/kernel/BeansTest.class \
         $(KERNEL_CLASSDIR)/sun/kernel/XMLTest.class \
         $(KERNEL_CLASSDIR)/sun/kernel/XMLTestSAX.class \
         $(KERNEL_CLASSDIR)/sun/kernel/ImageIconTest.class \
         $(KERNEL_CLASSDIR)/sun/kernel/SwingMinTest.class

$(KERNEL_TMPDIR)/SplitJRE.jar: classes $(KERNEL_CLASSDIR)/sun/jkernel ${KERNEL_TMPDIR}/dummy_bundle_1/classes.pack ${KERNEL_TMPDIR}/dummy_bundle_2/classes.pack
	$(CP) -f $(KERNEL_SRCDIR)/sun/kernel/bundles.xml $(KERNEL_CLASSDIR)/sun/kernel
	$(CP) -f $(HOTSPOT_KERNEL_PATH)/jvm.dll $(KERNEL_CLASSDIR)
	$(CP) -f $(HOTSPOT_KERNEL_PATH)/Xusage.txt $(KERNEL_CLASSDIR)
	$(CP) -f $(DEPLOY_TOPDIR)/src/kernel/tmp/jvm.cfg $(KERNEL_CLASSDIR)
	$(CP) -f $(OUTPUTDIR)/bin/jkernel.dll $(KERNEL_CLASSDIR)
# Maybe conditionally do a jar uf to add extra compression after the cfm?
ifdef EXTRA_COMP_LIB_PATH
	$(CP) -f $(EXTRA_COMP_LIB_PATH) $(KERNEL_TMPDIR)
	$(CP) -f $(EXTRA_COMP_LIB_PATH) $(OUTPUTDIR)/j2re-image/lib/deploy
	$(MKDIR) -p $(OUTPUTDIR)/lib/deploy
	$(CP) -f $(EXTRA_COMP_LIB_PATH) $(OUTPUTDIR)/lib/deploy
endif
	$(JAR) cfm $(KERNEL_TMPDIR)/SplitJRE.jar manifest.mf -C $(KERNEL_CLASSDIR) sun -C $(KERNEL_CLASSDIR) jkernel.dll \
		-C $(KERNEL_CLASSDIR) jvm.dll -C $(KERNEL_CLASSDIR) jvm.cfg \
		-C $(KERNEL_TMPDIR) dummy_bundle_1/classes.pack  \
		$(EXTRA_COMP_JAR_FLAG) \
		-C $(KERNEL_TMPDIR) dummy_bundle_2/classes.pack

%.pack: %.jar
	$(ABS_OUTPUTDIR)/bin/pack200$(EXE_SUFFIX) --no-gzip $@ $<

${KERNEL_TMPDIR}/dummy:
	${MKDIR} $@

${KERNEL_TMPDIR}/dummy_bundle_1:
	${MKDIR} $@

${KERNEL_TMPDIR}/dummy_bundle_2:
	${MKDIR} $@

${KERNEL_TMPDIR}/dummy/sun/kernel/Dummy1.class: ${KERNEL_SRCDIR}/sun/kernel/Dummy1.java ${KERNEL_TMPDIR}/dummy
	$(JAVAC) -d $(KERNEL_TMPDIR)/dummy $<

${KERNEL_TMPDIR}/dummy_bundle_1/classes.jar: ${KERNEL_TMPDIR}/dummy/sun/kernel/Dummy1.class ${KERNEL_TMPDIR}/dummy_bundle_1
	${JAR} cf $@ -C ${KERNEL_TMPDIR}/dummy sun/kernel/Dummy1.class

${KERNEL_TMPDIR}/dummy/sun/kernel/Dummy2.class: ${KERNEL_SRCDIR}/sun/kernel/Dummy2.java ${KERNEL_TMPDIR}/dummy
	$(JAVAC) -d $(KERNEL_TMPDIR)/dummy $<

${KERNEL_TMPDIR}/dummy_bundle_2/classes.jar: ${KERNEL_TMPDIR}/dummy/sun/kernel/Dummy2.class ${KERNEL_TMPDIR}/dummy_bundle_2
	${JAR} cf $@ -C ${KERNEL_TMPDIR}/dummy sun/kernel/Dummy2.class

$(KERNEL_CLASSDIR)/%.class: $(KERNEL_SRCDIR)/%.java
	$(JAVAC) -d $(KERNEL_CLASSDIR) -cp ${KERNEL_SRCDIR} $?

.PHONY: all classes kernel-image

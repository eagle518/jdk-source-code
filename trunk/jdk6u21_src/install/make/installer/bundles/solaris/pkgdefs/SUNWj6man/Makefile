#
# @(#)Makefile	1.19 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

include	../Makefile.com

DATAFILES += $(PACKAGE_TMP_DIR)/depend

PKGFILES = man/man1

# Packages to list in the depend file (see ../Makefile.targ).
PKG_DEPENDENCIES = SUNWj6rt

all:	$(FILES) $(PACKAGE_TMP_DIR)/action

install: all generate-prototype make-package 

generate-prototype: $(PKG_ARCHIVE) FRC
	$(ECHO) d none $(REGISTRY_DIR) 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp;
	$(ECHO) d none $(REGISTRY_DIR)$(INSTANCE_DIR) 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp;
	$(ECHO) d none $(PKGDEST) 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp;
	$(ECHO) d none $(PKGDEST)/man 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp;
	$(ECHO) d none $(PKGDEST)/man/man1 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp;
	for i in `$(CD) $(JDK_IMAGE_DIR) ; $(FIND) $(PKGFILES) -type f | $(CAT)` ; do \
	    $(ECHO) f none $(PKGDEST)$$i/$$j=$(JDK_IMAGE_DIR)/$$i/$$j 0444 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp; \
	done;

include	../Makefile.targ

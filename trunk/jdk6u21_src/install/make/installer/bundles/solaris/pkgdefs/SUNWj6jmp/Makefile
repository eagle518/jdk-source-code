#
# @(#)Makefile	1.15 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

include	../Makefile.com

DATAFILES += $(PACKAGE_TMP_DIR)/depend

PKGFILES = \
		man/ja/man1 \
		man/ja_JP.PCK/man1 \
		man/ja_JP.UTF-8/man1

# Packages to list in the depend file (see ../Makefile.targ).
PKG_DEPENDENCIES = SUNWj6man

all:	$(FILES) $(PACKAGE_TMP_DIR)/action

install: all generate-prototype make-package

generate-prototype: $(PKG_ARCHIVE) FRC
	$(ECHO) d none $(REGISTRY_DIR) 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp;
	$(ECHO) d none $(REGISTRY_DIR)$(INSTANCE_DIR) 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp;
	$(ECHO) d none $(PKGDEST) 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp;
	$(ECHO) d none $(PKGDEST)/man 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp;
	$(ECHO) d none $(PKGDEST)/man/ja 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp;
	$(ECHO) d none $(PKGDEST)/man/ja_JP.PCK 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp;
	$(ECHO) d none $(PKGDEST)/man/ja_JP.UTF-8 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp;
	for i in $(PKGFILES); do \
	    $(ECHO) d none $(PKGDEST)$$i 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp; \
            for j in `$(CD) $(JDK_IMAGE_DIR) ; $(FIND) $$i -type f | $(CAT)` ; do \
		$(ECHO) f none $(PKGDEST)$$j=$(JDK_IMAGE_DIR)/$$j 0444 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp; \
	    done; \
	done;

include	../Makefile.targ

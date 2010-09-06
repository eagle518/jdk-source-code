#
# @(#)Makefile	1.10 04/06/10
#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

include	../Makefile.com

DATAFILES += $(PACKAGE_TMP_DIR)/depend

MAN_DEST_PATH = \
		man/ja/sman1 \
		man/ja_JP.PCK/sman1 \
		man/ja_JP.UTF-8/sman1

# Packages to list in the depend file (see ../Makefile.targ).
PKG_DEPENDENCIES = SUNWj5man

EXTRA_FILES =	javaman.ent

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
	for i in $(MAN_DEST_PATH); do \
	    $(ECHO) d none $(PKGDEST)$$i 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp; \
	    for j in $(MAN_PAGE_FILES) $(EXTRA_FILES); do \
		$(ECHO) f none $(PKGDEST)$$i/$$j=$(JDK_IMAGE_DIR)/$$i/$$j 0444 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp; \
	    done; \
	done;

include	../Makefile.targ

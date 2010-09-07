#
#  @(#)Makefile	1.7 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

include	../Makefile.com

DATAFILES += $(PACKAGE_TMP_DIR)/depend

PKGFILES = etc

# Packages to list in the depend file (see ../Makefile.targ).
PKG_DEPENDENCIES = SUNWj6rt SUNWmfrun

all:	$(FILES) $(PACKAGE_TMP_DIR)/action

install: all generate-prototype make-package

generate-prototype: $(PKG_ARCHIVE) FRC
	$(CP) ../common/i.preserve $(PACKAGE_TMP_DIR)
	$(CP) postinstall $(PACKAGE_TMP_DIR)
	$(CP) space $(PACKAGE_TMP_DIR)
	$(TOUCH) $(PACKAGE_TMP_DIR)/.system.lock
	$(TOUCH) $(PACKAGE_TMP_DIR)/.systemRootModFile

include	../Makefile.targ

#
# @(#)Makefile	1.2 10/05/12
#
# Copyright (c) 2010, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

include	../Makefile.com

PKGFILES =  var

all:	$(FILES) 

install: all generate-prototype make-package

generate-prototype: $(PKG_ARCHIVE) FRC
	$(GREP) "^#" ../common/depend > $(PACKAGE_TMP_DIR)/depend;
	$(CP) default_java $(PACKAGE_TMP_DIR);

include	../Makefile.targ

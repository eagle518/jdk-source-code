#
# @(#)Makefile.com	1.12 04/12/06
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

include		../Makefile.master

DATAFILES=	$(PACKAGE_TMP_DIR)/copyright

TMP_AWK=	$(TMP_DIR)/awk_pkginfo

FILES=		$(DATAFILES) $(PACKAGE_TMP_DIR)/pkginfo

PACKAGE=	$(notdir $(shell pwd))

CLOBBERFILES=	$(FILES) $(PACKAGE_TMP_DIR)/action $(PACKAGE_TMP_DIR)/pkginfo


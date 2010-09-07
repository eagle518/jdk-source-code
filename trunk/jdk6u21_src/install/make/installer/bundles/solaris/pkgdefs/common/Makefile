#
# @(#)Makefile	1.25 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# pkgdefs/Makefile
#

include		../Makefile.com

SYMLINK=	$(LN) -s

all:		$(TMP_AWK) 

install:	$(TMP_AWK) 

clean:
		$(RM) $(TMP_AWK)

clobber:	clean
		$(RM) -rf $(PKG_PROTO)


$(TMP_AWK):	bld_awk_pkginfo.ksh FRC
		$(MKDIR) -p $(TMP_DIR)
		$(KSH) ./bld_awk_pkginfo.ksh \
		    -p "$(RELEASE)/$(FULL_VERSION)" -o $@

FRC:

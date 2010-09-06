#
# @(#)Makefile	1.22 04/04/12
#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
		$(KSH) ./bld_awk_pkginfo.ksh -m $(ARCH) \
		    -p "$(RELEASE)/$(FULL_VERSION)" -o $@

FRC:

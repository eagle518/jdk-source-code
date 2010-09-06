#
# @(#)Makefile	1.30 04/06/10
#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

include	../Makefile.com

DATAFILES += $(PACKAGE_TMP_DIR)/depend

PKGFILES =	$(JDK_DOCS) LICENSE \
		bin \
		lib \
		src.zip \
		include

PKG_SLINK_DEP = jre

# Packages to list in the depend file (see ../Makefile.targ).
PKG_DEPENDENCIES = SUNWj5rt SUNWmfrun

all:	$(FILES) $(PACKAGE_TMP_DIR)/action

install: all generate-prototype make-package

#Search through the j2sdk-image and get all the bin, lib and 
#include directories and put them in the prototype file with 
#755 permissions. Then find all the dev files and give them 755
#permissions if they are executable, 644 otherwise.  Get all
#of the files in bin/*, except for those that are also in
#jre/bin/*.  Also get src.zip and LICENSE.  Finally, find
#all of the symlinks, get the values of each, and place them
#correctly in the prototype file with 555 permissions.
generate-prototype: $(PKG_ARCHIVE) FRC
	$(CP) postinstall $(PACKAGE_TMP_DIR)
	$(CP) space $(PACKAGE_TMP_DIR)
	$(ECHO) d none $(REGISTRY_DIR) 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp;
	$(ECHO) d none $(REGISTRY_DIR)$(INSTANCE_DIR) 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp;
	$(ECHO) d none $(PKGDEST) 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp;
	for i in `$(CD) $(JDK_IMAGE_DIR) ; $(FIND) $(PKGFILES) -type d | $(CAT)` ; do \
	      $(ECHO) d none $(PKGDEST)$$i 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp; \
	done;
	for i in `$(CD) $(JDK_IMAGE_DIR) ; $(FIND) $(PKGFILES) -type f | $(CAT)` ; do \
	    p=`$(FIND) $(JDK_IMAGE_DIR)/$$i \( -perm -0100 -o -perm -0010 -o -perm -0001 \) -print `; \
	    if [ -n "$$p" ]; then \
	      if [ ! -x $(JDK_IMAGE_DIR)/$(PKG_SLINK_DEP)/$$i ] ; then \
	        $(ECHO) f none $(PKGDEST)$$i=$(JDK_IMAGE_DIR)/$$i 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp; \
	      fi; \
	    else \
	        $(ECHO) f none $(PKGDEST)$$i=$(JDK_IMAGE_DIR)/$$i 644 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp; \
	    fi; \
	done;
	for i in `$(CD) $(JDK_IMAGE_DIR) ; $(FIND) $(PKGFILES) -type l | $(CAT)` ; do \
	    t=`$(LS) -l $(JDK_IMAGE_DIR)/$$i | $(SED) -e 's=.*-> =='`; \
	    if [ ! -h $(JDK_IMAGE_DIR)/$(PKG_SLINK_DEP)/$$i ]; then \
	      $(ECHO) s none $(PKGDEST)$$i=$$t 555 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp; \
	    fi; \
	done;

include	../Makefile.targ

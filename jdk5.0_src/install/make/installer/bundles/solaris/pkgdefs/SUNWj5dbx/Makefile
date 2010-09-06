#
# @(#)Makefile	1.13 04/06/10
#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

include	../Makefile.com

DATAFILES += $(PACKAGE_TMP_DIR)/depend

PKGDBGFILES =  bin jre

# Packages to list in the depend file (see ../Makefile.targ).
PKG_DEPENDENCIES = SUNWj5dbg SUNWj5rtx 

all:	$(FILES) $(PACKAGE_TMP_DIR)/action

install: all generate-prototype make-package

#Search through the j2sdk-debug-image and get all directories and 
#put them in the prototype file with 755 permissions.  Then find
#all the files and give them 755 permissions if they are 
#executable, 644 otherwise.
generate-prototype: $(PKG_ARCHIVE) FRC
	$(ECHO) d none $(REGISTRY_DIR) 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp;
	$(ECHO) d none $(REGISTRY_DIR)$(INSTANCE_DIR) 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp;
	$(ECHO) d none $(PKGDEST) 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp;
	for i in `$(CD) $(JDK_DEBUG_IMAGE_DIR) ; $(FIND) $(PKGDBGFILES) -type d | $(CAT)` ; do \
	    $(ECHO) d none $(PKGDEST)$$i 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp; \
	done ;
	for i in `$(CD) $(JDK_DEBUG_IMAGE_DIR) ; $(FIND) * -type f | $(CAT)` ; do \
	    p=`$(FIND) $(JDK_DEBUG_IMAGE_DIR)/$$i \( -perm -0100 -o -perm -0010 -o -perm -0001 \) -print `; \
	    if [ -n "$$p" ] ; then \
	        $(ECHO) f none $(PKGDEST)$$i=$(JDK_DEBUG_IMAGE_DIR)/$$i 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp; \
	    else \
	        $(ECHO) f none $(PKGDEST)$$i=$(JDK_DEBUG_IMAGE_DIR)/$$i 644 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp; \
	    fi; \
	done ;

include	../Makefile.targ

#
# @(#)Makefile	1.22 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

include	../Makefile.com

DATAFILES += $(PACKAGE_TMP_DIR)/depend

PKGFILES =  jre

# Packages to list in the depend file (see ../Makefile.targ).
PKG_DEPENDENCIES = SUNWj6rt

POSTCOMMON = ../common/postcommon

all:	$(FILES) $(PACKAGE_TMP_DIR)/action

install: all generate-prototype make-package

#Search through the j2sdk-image and get all the jre directories 
#and put them in the prototype file with 755 permissions.  Then
#find all the jre files and give them 755 permissions if they are 
#executable, 644 otherwise.  For each file in jre/bin* also get
#the same file in bin/*.
#Find all of the symlinks get the values of each,
#and place them correctly in the prototype file with 555 permissions.
generate-prototype: $(PKG_ARCHIVE) FRC
	$(CAT) $(POSTCOMMON) postinstall > $(PACKAGE_TMP_DIR)/postinstall
	$(CAT) $(POSTCOMMON) postremove > $(PACKAGE_TMP_DIR)/postremove
	$(ECHO) d none $(REGISTRY_DIR) 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp;
	$(ECHO) d none $(REGISTRY_DIR)$(INSTANCE_DIR) 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp;
	$(ECHO) d none $(PKGDEST) 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp;
	$(ECHO) d none $(PKGDEST)bin 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp;
	$(ECHO) d none $(PKGDEST)bin/$(ARCH) 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp;
	for i in `$(CD) $(JDK_IMAGE_DIR) ; $(FIND) $(PKGFILES) -type d | $(CAT)` ; do \
	      $(ECHO) d none $(PKGDEST)$$i 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp; \
	done;
	for i in `$(CD) $(JDK_IMAGE_DIR) ; $(FIND) $(PKGFILES) -type f | $(CAT)` ; do \
	    p=`$(FIND) $(JDK_IMAGE_DIR)/$$i \( -perm -0100 -o -perm -0010 -o -perm -0001 \) -print `; \
	    if [ -n "$$p" ] ; then \
	       $(ECHO) f none $(PKGDEST)$$i=$(JDK_IMAGE_DIR)/$$i 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp; \
	       b=`$(BASENAME) $$p`; \
	       if [ -x $(JDK_IMAGE_DIR)/bin/$(ARCH)/$$b ] ; then \
		 $(ECHO) f none $(PKGDEST)bin/$(ARCH)/$$b=$(JDK_IMAGE_DIR)/bin/$(ARCH)/$$b 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp; \
	       fi; \
	    else \
	       $(ECHO) f none $(PKGDEST)$$i=$(JDK_IMAGE_DIR)/$$i 644 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp; \
	    fi; \
	done;
	for i in `$(CD) $(JDK_IMAGE_DIR) ; $(FIND) $(PKGFILES) -type l | $(CAT)` ; do \
	    t=`$(LS) -l $(JDK_IMAGE_DIR)/$$i | $(SED) -e 's=.*-> =='`; \
	    $(ECHO) s none $(PKGDEST)$$i=$$t 555 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp; \
	done;

include	../Makefile.targ

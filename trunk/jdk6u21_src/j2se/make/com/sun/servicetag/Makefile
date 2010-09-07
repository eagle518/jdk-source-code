#
# @(#)Makefile	1.4 10/03/23
#
# Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../../..
PACKAGE = com.sun.servicetag
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile
#
AUTO_FILES_JAVA_DIRS = com/sun/servicetag 

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

SERVICETAG_LIBDIR = $(LIBDIR)/servicetag
SERVICETAG_RESOURCES_DIR = $(CLASSDESTDIR)/com/sun/servicetag/resources
FILES_copy = $(SERVICETAG_RESOURCES_DIR)/javase_5_swordfish.properties \
             $(SERVICETAG_RESOURCES_DIR)/javase_6_swordfish.properties \
             $(SERVICETAG_RESOURCES_DIR)/product_registration.xsd \
             $(SERVICETAG_RESOURCES_DIR)/register.html \
             $(SERVICETAG_RESOURCES_DIR)/register_ja.html \
             $(SERVICETAG_RESOURCES_DIR)/register_zh_CN.html \
             $(SERVICETAG_LIBDIR)/jdk_header.png

#
#OTHER_JAVACFLAGS += -Xlint:unchecked
                                                         
build: install-servicetag-lib copy-files

copy-files: $(FILES_copy)

$(CLASSBINDIR)/%: $(SHARE_SRC)/classes/%
	$(install-file)

$(SERVICETAG_LIBDIR)/jdk_header.png: $(SHARE_SRC)/classes/com/sun/servicetag/resources/jdk_header.png
	$(install-file)
	$(CHMOD) 444 $@

install-servicetag-lib:
	@$(RM) -rf $(SERVICETAG_LIBDIR)
	$(MKDIR) $(SERVICETAG_LIBDIR)
                                                                                                            
clean clobber::
	@$(RM) $(FILES_copy)
                            
.PHONY: copy-files

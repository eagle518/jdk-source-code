#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# @(#)Makefile	1.20 07/04/03
#
#
# This makefile will create the jinstall.inf and jinstall.xml file
# Unique target : all
#


# get a standard build environment set...
INSTALL_BUILDDIR = ../../../..
PRODUCT = cab

TEMPDIR = $(TEMP_DIR)/$(PRODUCT)

include $(INSTALL_BUILDDIR)/common/Defs.gmk

###################### jinstallcab ########################
#
# Create the jinstall.cab file using ishield8's lzx compression
# This will overwrite the File.idt table for the jre, so that the 
# jinstall.exe and jinstall.inf file are the only 2 files 
# that our part of the jre product.  After building iftw media, we
# are then left with a lzx'ed compressed jm*cab file
# which will become jinstall.cab
jinstallcab: 
ifeq ($(ARCH_DATA_MODEL),32)
	$(MKDIR) -p $(TEMPDIR)
	$(CP) $(TEMP_DIR)/jinstall-iftw/obj/jinstall_iftw.exe $(TEMPDIR)/jinstall.exe
	$(CP) $(ALT_OUTPUTDIR)/tmp/xmlinffile/jinstall-$(JDK_MKTG_UNDERSCORE_VERSION).inf $(TEMPDIR)/
	$(CD) $(TEMPDIR)/; $(DEVTOOLS_PATH)cabarc -m LZX:21 n $(BIN_BUNDLEDIR)/jinstall-$(BIN_BUNDLE_NAMEPART)-$(BUNDLE_DATE).cab *
    ifneq ($(J4B),true)
        #Build java.com cab file with "-rv.cab" extension
	$(RM) $(TEMPDIR)/*
	$(CP) $(TEMP_DIR)/jinstall-iftw-rv/obj/jinstall_iftw_rv.exe $(TEMPDIR)/jinstall_jvector.exe
	$(CP) $(ALT_OUTPUTDIR)/tmp/xmlinffile/jinstall-$(JDK_MKTG_UNDERSCORE_VERSION)-rv.inf $(TEMPDIR)/jinstall-$(JDK_MKTG_UNDERSCORE_VERSION).inf
	$(CD) $(TEMPDIR)/; $(DEVTOOLS_PATH)cabarc -m LZX:21 n $(BIN_BUNDLEDIR)/jinstall-$(BIN_BUNDLE_NAMEPART)-$(BUNDLE_DATE)-rv.cab *
    endif
    ifeq ($(SIGN_FLAG),1)
	$(call Try3Times, $(FULL_SIGNER) -n "Java(TM) SE Runtime Environment $(JDK_MINOR_VERSION)$(MARKET_NAME)" -i http://java.com/ $(BIN_BUNDLEDIR)/jinstall-$(BIN_BUNDLE_NAMEPART)-$(BUNDLE_DATE).cab )
        ifneq ($(J4B),true)
	    $(call Try3Times, $(FULL_SIGNER) -n "Java(TM) SE Runtime Environment $(JDK_MINOR_VERSION)$(MARKET_NAME)" -i http://java.com/ $(BIN_BUNDLEDIR)/jinstall-$(BIN_BUNDLE_NAMEPART)-$(BUNDLE_DATE)-rv.cab )
        endif
    endif
endif

###################### all ################################
# 
# THIS IS THE ONLY VALID TARGET (with clean). Copy the installation scripts
# and jre-image directory to the TEMP directory so we can delete/add
# files between the English and International installations. Do the two
# installations in succession.
# 
all: jinstallcab

###################### clean #############################
#
# Remove all produced files.
#
clean: 
	$(RM) -r $(TEMPDIR)

$(BIN_BUNDLEDIR):
	$(MKDIR) -p $@

.PHONY: all clean jinstallcab

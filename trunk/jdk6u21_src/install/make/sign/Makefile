#
#  @(#)Makefile	1.25 10/04/15
#
# Copyright (c) 2006, 2010, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
#
# Makefile for signing the Deployment Toolkit dlls
#

INSTALL_BUILDDIR=..
include $(INSTALL_BUILDDIR)/common/Defs.gmk

FULL_SIGNER_CMD=$(FULL_SIGNER) -n "Java(TM) SE Runtime Environment $(JDK_MINOR_VERSION)$(MARKET_NAME)" -i http://java.com/

all:: 

all build:: sign

sign::
ifeq ($(SIGN_FLAG),1)
	@# sign plugin dlls 
	$(call Try3Times, $(FULL_SIGNER) -n "Deployment Toolkit" -i http://java.com/ $(JRE_IMAGE_DIR)/bin/deployJava1.dll )
	$(call Try3Times, $(FULL_SIGNER) -n "Deployment Toolkit" -i http://java.com/ $(JRE_IMAGE_DIR)/bin/npdeployJava1.dll )
	$(call Try3Times, $(FULL_SIGNER_CMD) $(JRE_IMAGE_DIR)/bin/ssv.dll )
	$(call Try3Times, $(FULL_SIGNER_CMD) $(JRE_IMAGE_DIR)/bin/npjpi$(JRE_NODOTVERSION).dll )
	$(call Try3Times, $(FULL_SIGNER_CMD) $(JRE_IMAGE_DIR)/bin/jp2ssv.dll )
	$(call Try3Times, $(FULL_SIGNER_CMD) $(JRE_IMAGE_DIR)/bin/jp2iexp.dll )
	$(call Try3Times, $(FULL_SIGNER_CMD) $(JRE_IMAGE_DIR)/bin/wsdetect.dll )
	$(call Try3Times, $(FULL_SIGNER_CMD) $(JRE_IMAGE_DIR)/lib/deploy/jqs/ie/jqs_plugin.dll )
	@# sign all executables 
	@for i in `$(CD) $(JRE_IMAGE_DIR)/bin ; $(LS) *exe` ; do \
	    if [ "$$i" != "jucheck.exe" ]; then \
	        $(call Try3Times, $(FULL_SIGNER_CMD) $(JRE_IMAGE_DIR)/bin/"$$i" ) ; \
	    fi ; \
	done

endif

clean:
clobber:


# this should be the last rule in this file:
all::
	@$(ECHO) $(ARCH) "sign finished:  " $(FULL_VERSION)
#
# Phonies to avoid accidents.
#
.PHONY: all sign clean clobber

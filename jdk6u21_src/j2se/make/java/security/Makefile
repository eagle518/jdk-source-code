#
# @(#)Makefile	1.55 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
PACKAGE = java.security
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Rules
#
AUTO_FILES_JAVA_DIRS = java/security

#
# Directories
#

PROPS_SRC   = $(TOPDIR)/src/share/lib/security/java.security
SUNPKCS11   =

ifeq ($(PLATFORM), solaris)
PROPS_SRC   = $(TOPDIR)/src/share/lib/security/java.security-solaris
SUNPKCS11   = sunpkcs11

else # PLATFORM

# Register Microsoft CryptoAPI provider only on (non-64-bit) Windows platform.
ifeq ($(PLATFORM), windows)
  ifneq ($(ARCH_DATA_MODEL), 64)
    PROPS_SRC   = $(TOPDIR)/src/share/lib/security/java.security-windows
  endif
endif
endif # PLATFORM


PROPS_BUILD = $(LIBDIR)/security/java.security

SUNPKCS11_SRC   = $(TOPDIR)/src/share/lib/security/sunpkcs11-solaris.cfg
SUNPKCS11_BUILD = $(LIBDIR)/security/sunpkcs11-solaris.cfg

POLICY_SRC =	$(TOPDIR)/src/share/lib/security/java.policy
POLICY_BUILD = $(LIBDIR)/security/java.policy

CACERTS_SRC =	$(CACERTS_FILE)
CACERTS_BUILD = $(LIBDIR)/security/cacerts

BLACKLIST_SRC = $(TOPDIR)/src/share/lib/security/blacklist
BLACKLIST_BUILD = $(LIBDIR)/security/blacklist

TRUSTEDLIBS_SRC = $(TOPDIR)/src/share/lib/security/trusted.libraries
TRUSTEDLIBS_BUILD = $(LIBDIR)/security/trusted.libraries

FILES_class = $(FILES_java:%.java=$(CLASSBINDIR)/%.class)

#
# Rules
#
include $(BUILDDIR)/common/Rules.gmk

build: properties $(SUNPKCS11) policy cacerts blacklist trustedlibs

install: all

properties: classes  $(PROPS_BUILD)

sunpkcs11: classes $(SUNPKCS11_BUILD)

policy: classes  $(POLICY_BUILD)

cacerts: classes $(CACERTS_BUILD)

blacklist: classes  $(BLACKLIST_BUILD)

trustedlibs: classes  $(TRUSTEDLIBS_BUILD)

$(PROPS_BUILD): $(PROPS_SRC)
	$(install-file)

$(SUNPKCS11_BUILD): $(SUNPKCS11_SRC)
	$(install-file)

$(POLICY_BUILD): $(POLICY_SRC)
	$(install-file)

$(CACERTS_BUILD): $(CACERTS_SRC)
	$(install-file)

$(BLACKLIST_BUILD): $(BLACKLIST_SRC)
	$(install-file)

$(TRUSTEDLIBS_BUILD): $(TRUSTEDLIBS_SRC)
	$(install-file)

clean clobber:: .delete.classlist
	$(RM) -r $(CLASSBINDIR)/java/security
	$(RM) $(PROPS_BUILD) $(POLICY_BUILD) $(CACERTS_BUILD) $(SUNPKCS11_BUILD) $(BLACKLIST_BUILD) $(TRUSTEDLIBS_BUILD)

# Additional Rule for building sun.security.util
$(CLASSBINDIR)/%.class: $(SHARE_SRC)/sun/%.java
	@$(ECHO) $? >> $(TEMPDIR)/.classes.list


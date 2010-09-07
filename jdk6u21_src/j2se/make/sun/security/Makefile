#
# @(#)Makefile	1.61 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Build classes for security tools:
#
#		* keytool
#		* jarsigner
#		* policytool
#
# Also install wrappers for all of these.
#

BUILDDIR = ../..
include $(BUILDDIR)/common/Defs.gmk

ifdef J2RE_ONLY
TOOLS_SUBDIRS =
else 
TOOLS_SUBDIRS = jarsigner policytool
ifeq ($(PLATFORM), windows)
    TOOLS_SUBDIRS += kinit klist ktab
endif  # solaris
endif

# build sun/security/jgss/wrapper on non-windows platform
JGSS_WRAPPER =
ifneq ($(PLATFORM), windows)
    JGSS_WRAPPER = jgss/wrapper
endif

# Build PKCS#11 on all platforms except 64-bit Windows.
# We exclude windows-amd64 because we don't have any
# 64-bit PKCS#11 implementations to test with on that platform.
PKCS11 = pkcs11
ifeq ($(ARCH_DATA_MODEL), 64)
  ifeq ($(PLATFORM), windows)
    PKCS11 =
  endif
endif

# Build Microsoft CryptoAPI provider only on (non-64-bit) Windows platform.
MSCAPI =
ifeq ($(PLATFORM), windows)
  ifneq ($(ARCH_DATA_MODEL), 64)
    MSCAPI = mscapi
  endif
endif

SUBDIRS = acl action util x509 pkcs provider tools keytool jgss validator \
    jca rsa ec timestamp smartcardio \
    $(PKCS11) $(TOOLS_SUBDIRS) $(JGSS_WRAPPER) $(MSCAPI)

all build clean clobber::
	$(SUBDIRS-loop)

clean::
	$(RM) -r $(CLASSBINDIR)/sun/security

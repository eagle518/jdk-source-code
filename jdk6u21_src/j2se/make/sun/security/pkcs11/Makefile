#
# @(#)Makefile	1.11 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../../..
PACKAGE = sun.security.pkcs11
LIBRARY = j2pkcs11
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# C and Java Files
#
include FILES_c.gmk

AUTO_FILES_JAVA_DIRS = sun/security/pkcs11

#
# Java files that define native methods
#
FILES_export = \
    sun/security/pkcs11/wrapper/PKCS11.java \
    sun/security/pkcs11/Secmod.java

#
# Find native code
#
vpath %.c \
  $(SHARE_SRC)/native/sun/security/pkcs11/wrapper \
  $(PLATFORM_SRC)/native/sun/security/pkcs11/wrapper

#
# Find include files
#
OTHER_INCLUDES += \
  -I$(SHARE_SRC)/native/sun/security/pkcs11/wrapper \
  -I$(PLATFORM_SRC)/native/sun/security/pkcs11/wrapper

#
# Rules
#
CLASSDESTDIR = $(TEMPDIR)/classes

include $(BUILDDIR)/common/Mapfile-vers.gmk

include $(BUILDDIR)/common/Library.gmk

#
# Libraries to link
#
ifeq ($(PLATFORM), windows)
  OTHER_LDLIBS = $(JVMLIB)
else
  OTHER_LDLIBS = -ldl $(JVMLIB)
endif

PKCS11_JAR_SRC = $(BUILDDIR)/tools/crypto/pkcs11/sunpkcs11.jar
PKCS11_JAR_DST = $(EXTDIR)/sunpkcs11.jar
PKCS11_JAR_TMP = $(TEMPDIR)/sunpkcs11.jar

clobber::
	-$(RM) $(PKCS11_JAR_DST) $(PKCS11_JAR_TMP)

all: classes precompiledjar

precompiledjar: $(PKCS11_JAR_DST)

$(PKCS11_JAR_DST): $(PKCS11_JAR_SRC)
	$(install-file)

jar: classes 
	-$(RM) $(PKCS11_JAR_DST) $(PKCS11_JAR_TMP)
	$(JAR) cf $(PKCS11_JAR_TMP) -C $(CLASSDESTDIR) sun/security/pkcs11 \
	     $(JAR_JFLAGS)
	$(CP) $(PKCS11_JAR_TMP) $(PKCS11_JAR_DST)
	@$(java-vm-cleanup)

# testing only
KEY_LOCATION	= /security/ws/JCE-mustang/src
KEYSTORE	= $(KEY_LOCATION)/JCEKeyStore.ks
PASSPHRASE	= $(KEY_LOCATION)/passPhrase
ALIAS		= jce_rsa

sign: jar
	$(JAR)signer -keystore $(KEYSTORE) $(PKCS11_JAR_DST) $(ALIAS) < $(PASSPHRASE)
	@$(ECHO)
	@$(java-vm-cleanup)

.PHONY: precompiledjar jar


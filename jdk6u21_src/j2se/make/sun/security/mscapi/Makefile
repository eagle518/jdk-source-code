#
# @(#)Makefile	1.4 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../../..
PACKAGE = sun.security.mscapi
LIBRARY = sunmscapi
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

CPLUSPLUSLIBRARY=true

#
# C++ and Java Files
#
include FILES_cpp.gmk

AUTO_FILES_JAVA_DIRS = sun/security/mscapi

#
# Java files that define native methods
#
FILES_export = \
    sun/security/mscapi/KeyStore.java \
    sun/security/mscapi/Key.java \
    sun/security/mscapi/PRNG.java \
    sun/security/mscapi/RSACipher.java \
    sun/security/mscapi/RSAPublicKey.java \
    sun/security/mscapi/RSASignature.java \
    sun/security/mscapi/RSAKeyPairGenerator.java

#
# Find native code
#
vpath %.cpp \
  $(PLATFORM_SRC)/native/sun/security/mscapi

#
# Find include files
#
OTHER_INCLUDES += \
  -I$(PLATFORM_SRC)/native/sun/security/mscapi

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
  OTHER_LDLIBS += $(JVMLIB) Crypt32.Lib
endif

MSCAPI_JAR_SRC = $(BUILDDIR)/tools/crypto/mscapi/sunmscapi.jar
MSCAPI_JAR_DST = $(EXTDIR)/sunmscapi.jar
MSCAPI_JAR_TMP = $(TEMPDIR)/sunmscapi.jar

clobber::
	-$(RM) $(MSCAPI_JAR_DST) $(MSCAPI_JAR_TMP)

all: classes precompiledjar

precompiledjar: $(MSCAPI_JAR_DST)

$(MSCAPI_JAR_DST): $(MSCAPI_JAR_SRC)
	$(install-file)

jar: classes
	-$(RM) $(MSCAPI_JAR_DST) $(MSCAPI_JAR_TMP)
	$(JAR) cf $(MSCAPI_JAR_TMP) -C $(CLASSDESTDIR) sun/security/mscapi \
	     $(JAR_JFLAGS)
	$(CP) $(MSCAPI_JAR_TMP) $(MSCAPI_JAR_DST)
	@$(java-vm-cleanup)

# testing only
KEY_LOCATION	= /security/ws/JCE-mustang/src
KEYSTORE	= $(KEY_LOCATION)/JCEKeyStore.ks
PASSPHRASE	= $(KEY_LOCATION)/passPhrase
ALIAS		= jce_rsa

sign: jar
	$(JAR)signer -keystore $(KEYSTORE) $(MSCAPI_JAR_DST) $(ALIAS) < $(PASSPHRASE)
	@$(ECHO)
	@$(java-vm-cleanup)

.PHONY: precompiledjar jar

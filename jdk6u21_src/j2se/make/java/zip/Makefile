#
# @(#)Makefile	1.45 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#


BUILDDIR = ../..
PACKAGE = java.util.zip
LIBRARY = zip
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

ZLIB_VERSION = 1.1.3

#
# Files to compile.
#
include FILES_c.gmk
AUTO_FILES_JAVA_DIRS = java/util/zip

FILES_export = \
	java/util/zip/CRC32.java \
	java/util/zip/Adler32.java \
	java/util/zip/Inflater.java \
	java/util/zip/Deflater.java \
	java/util/zip/ZipFile.java \
	java/util/zip/ZipEntry.java \
	java/util/jar/JarFile.java

ifneq ($(PLATFORM), windows)
OTHER_CFLAGS += -DUSE_MMAP
else
  # Files built here do not compile with warning level 3 if warnings are fatal
  COMPILER_WARNINGS_FATAL=false
endif

#
# Library to compile.
#
ifeq ($(PLATFORM), solaris)
  ifneq ($(ARCH), amd64)
    FILES_reorder += reorder-$(ARCH)
  endif
endif
include $(BUILDDIR)/common/Mapfile-vers.gmk
include $(BUILDDIR)/common/Library.gmk

#
# Bootstrap stage.
#
JAVAC_CMD = $(JAVAC_BOOT_CMD)
JAVAH_CMD = $(JAVAH_BOOT_CMD)

ifeq ($(PLATFORM), windows)
OTHER_LCF = -export:ZIP_Open -export:ZIP_Close -export:ZIP_FindEntry \
            -export:ZIP_ReadEntry -export:ZIP_GetNextEntry
else
CPPFLAGS += -UDEBUG 
endif

CPPFLAGS += -I$(SHARE_SRC)/native/java/util/zip/zlib-$(ZLIB_VERSION)
CPPFLAGS += -I$(SHARE_SRC)/native/java/io
CPPFLAGS += -I$(PLATFORM_SRC)/native/java/io

#
# Add to ambient vpath so we pick up the library files
#
vpath %.c $(SHARE_SRC)/native/$(PKGDIR)/zlib-$(ZLIB_VERSION)

#
# Has been converted to the JNI: generate JNI-style header files
#
JAVAHFLAGS += -jni

#
# Link to JVM library for JVM_Zip* functions and for dlsym (where necessary)
#
OTHER_LDLIBS = $(JVMLIB)
ifeq ($(PLATFORM) $(ARCH_DATA_MODEL),solaris 32)
  OTHER_LDLIBS += -ldl
endif


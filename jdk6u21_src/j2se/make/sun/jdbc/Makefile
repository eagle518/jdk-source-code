#
# @(#)Makefile	1.46 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for JDBC-ODBC Bridge Driver 
#
# Note - the native library for the bridge is linked with the 
# shared library for the ODBC driver manager.  
#

BUILDDIR = ../..
PACKAGE = sun.jdbc.odbc
LIBRARY = JdbcOdbc
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#

AUTO_FILES_JAVA_DIRS = sun/jdbc/odbc

FILES_c = \
	JdbcOdbc.c

FILES_export = \
    sun/jdbc/odbc/JdbcOdbc.java 

ifeq ($(PLATFORM),linux)
    # on linux you may not have an odbc library
     INIT = make_libs
endif

ifeq ($(PLATFORM),windows)
  # Files built here do not compile with warning level 3 if warnings are fatal
  COMPILER_WARNINGS_FATAL=false
endif

#
# Rules
#
  include $(BUILDDIR)/common/Library.gmk

#
# The UNIX define specifies conditional compilation for UNIX
#
ifneq ($(PLATFORM), windows)
  CFLAGS += -DUNIX
endif 

ifeq ($(PLATFORM), linux)
    LDFLAGS += -L$(TEMPDIR) -lodbcinst -lodbc
endif

ifeq ($(PLATFORM), solaris)
  ifeq ($(ARCH_DATA_MODEL), 64)
    LDFLAGS += -L$(ODBCDIR)/ISLIodbc/2.11/lib -z nodefs
  else
    LDFLAGS += -L$(ODBCDIR)/ISLIodbc/2.11/lib -lodbcinst -lodbc
  endif
endif

ifeq ($(PLATFORM), windows)
    LDLIBS += odbc32.lib odbccp32.lib
endif 

#
# The native code for the bridge uses conditional compilation to 
# support Solaris, Win95 and Mac PPC. This is the path to the shared C files
# (which unfortunately are in the same directory as shared Java files).
#
vpath %.c $(SHARE_SRC)/classes/sun/jdbc/odbc

#
# Has been converted to the JNI: generate JNI-style header files
#
JAVAHFLAGS += -jni

ifeq ($(PLATFORM), linux)

make_libs: $(TEMPDIR)/libodbcinst.so $(TEMPDIR)/libodbc.so

$(TEMPDIR)/libodbcinst.so: dummyodbc.c  $(TEMPDIR)
	$(CC) -shared -o $@ $<

$(TEMPDIR)/libodbc.so: dummyodbc.c $(TEMPDIR) 
	$(CC) -shared -o $@ $<

clean::
	$(RM) -f $(TEMPDIR)/libodbcinst.so $(TEMPDIR)/libodbc.so

endif

#
# Files that need to be copied
#
SERVICEDIR = $(CLASSBINDIR)/META-INF/services

FILES_copy = \
	$(SERVICEDIR)/java.sql.Driver


FILES_mkdirs = \
	$(CLASSBINDIR)/META-INF \
	$(CLASSBINDIR)/META-INF/services

FILES_copydirs = \
	$(CLASSBINDIR) \
	$(FILES_mkdirs)

build: copy-files

copy-files: $(FILES_copy)

$(SERVICEDIR)/%: $(SHARE_SRC)/classes/sun/jdbc/odbc/META-INF/services/%
	$(install-file)

clean clobber::
	$(RM) $(FILES_copy)

.PHONY: copy-files


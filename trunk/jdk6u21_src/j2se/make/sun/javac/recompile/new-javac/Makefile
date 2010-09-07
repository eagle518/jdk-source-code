#
# @(#)Makefile	1.22 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Re-compile newjavac classes possibly built by bootstrap compiler.
#

BUILDDIR = ../../../..
PACKAGE = com.sun.tools.javac
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files-to-compile.
#
include $(BUILDDIR)/sun/javac/javac/FILES_java.gmk

#
# Overwrite destination directory.
#
ALT_CLASSBINDIR = $(TEMPDIR)/newjavac/newclasses
ALT_CLASSDESTDIR := $(CLASSDESTDIR)
CLASSDESTDIR    = $(ALT_CLASSBINDIR)
CLASSBINDIR     = $(ALT_CLASSBINDIR)

JAVAC_CLS_FLAG     = -classpath $(CLASSBINDIR)/classes

### This is a horrible hack to make sure sourceless classes like Normalizer.class can be found.
JAVAC_CLS_FLAG     += -bootclasspath $(JAVAC_BCP)

#
# Rules.
#
include $(BUILDDIR)/common/Classes.gmk

# Don't leave old classes lying around in 'newclasses' !

clean clobber::
	$(RM) -r $(TEMPDIR)/newjavac/newclasses

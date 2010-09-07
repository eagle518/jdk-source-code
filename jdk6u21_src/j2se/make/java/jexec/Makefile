#
# @(#)Makefile	1.20 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building jexec, a wrapper for direct execution of JAR files
# on UNIX systems.
#

BUILDDIR = ../..

PACKAGE = java.jexec
PRODUCT = sun
PROGRAM = jexec

include $(BUILDDIR)/common/Defs.gmk

include $(BUILDDIR)/common/Rules.gmk

JEXE_SRC =      $(PLATFORM_SRC)/bin

FILES_c =       jexec.c
FILES_o =       $(patsubst %.c,$(OBJDIR)/%.o,$(FILES_c))

vpath %.c       $(JEXE_SRC)

ifeq ($(PLATFORM), linux)
  #
  # On Linux jexec goes in lib, not lib/<arch>
  #
  LIB_LOCATION = $(LIBDIR)

  #
  # Also include the manifest_info.h header file.
  #
  OTHER_INCLUDES += -I$(SHARE_SRC)/bin
endif # PLATFORM

build: $(LIB_LOCATION)/$(PROGRAM)

$(LIB_LOCATION)/$(PROGRAM):: $(FILES_o)
	$(prep-target)
	$(LINK_PRE_CMD) $(CC) $(LDFLAGS) $(FILES_o) $(CC_OBJECT_OUTPUT_FLAG)$@

clean clobber:: 
	$(RM) $(FILES_o) $(LIB_LOCATION)/$(PROGRAM)


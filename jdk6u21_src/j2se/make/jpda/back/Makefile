#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.43 10/03/23
#
# Makefile for building the JDI back-end implementation
#

BUILDDIR = ../..
LIBRARY = jdwp
PRODUCT = jpda

FILES_m = mapfile-vers

include $(BUILDDIR)/common/Defs.gmk

# Use the mapfile-vers (See the mapfile located with this Makefile)
ifdef FILES_m
include $(BUILDDIR)/common/Mapfile-vers.gmk
endif

OTHER_INCLUDES = -I$(SHARE_SRC)/transport/export \
                 -I$(SHARE_SRC)/back/export \
                 -I$(SHARE_SRC)/npt \
                 -I$(PLATFORM_SRC)/npt \
                 -I$(SHARE_SRC)/back \
                 -I$(PLATFORM_SRC)/back \
                 -I$(TEMPDIR)/../com.sun.tools.jdwpgen/jdwp

ifeq ($(PLATFORM), windows)
  OTHER_LDLIBS += $(JVMLIB)
endif # PLATFORM

ifeq ($(PLATFORM), linux)
  # fork1() is declared in libjvm.so
  OTHER_LDLIBS += -ldl $(JVMLIB)
endif # PLATFORM

ifeq ($(PLATFORM), solaris)
  OTHER_LDLIBS += -ldl
  # Use of lint has been helpful, but lots of errors needed to be suppressed:
  #OTHER_LINTFLAGS += -errtags=yes
  # The following error tags are all errors that we suppress right now
  OTHER_LINTFLAGS += -erroff=E_CONST_EXPR
  OTHER_LINTFLAGS += -erroff=E_CONSTANT_CONDITION
  OTHER_LINTFLAGS += -erroff=E_SUPPRESSION_DIRECTIVE_UNUSED
  OTHER_LINTFLAGS += -erroff=E_INCL_NUSD
  OTHER_LINTFLAGS += -erroff=E_H_C_CHECK0
  # We only care about errors in user include files
  OTHER_LINTFLAGS += -errhdr=%user
  # Logging gives lint fits, so we turn it off
  OTHER_LINTFLAGS += -DNO_JDWP_LOGGING
  # We don't use the -Ncheck feature yet, it seems somewhat broken
  #OTHER_LINTFLAGS += -Ncheck -Nlevel=2
  # Even with above suppression, some files/code are hopelessly non-lintable
  #   signal.h has some old style C function prototypes in it (Solaris 9u2)
  MY_LINT_FILTER += | $(GREP) -v /usr/include
endif # PLATFORM

#
# This turns off all assert() checking in the optimized library
#
CPPFLAGS_OPT += -DNDEBUG

#
# This controls the ability to do logging in the library.
#
CPPFLAGS_DBG += -DJDWP_LOGGING
CPPFLAGS_OPT += -DJDWP_LOGGING

#
# Files to compile.
#
FILES_c = \
        $(CTARGDIR)ArrayReferenceImpl.c \
        $(CTARGDIR)ArrayTypeImpl.c \
        $(CTARGDIR)ClassTypeImpl.c \
        $(CTARGDIR)ClassLoaderReferenceImpl.c \
        $(CTARGDIR)ClassObjectReferenceImpl.c \
        $(CTARGDIR)EventRequestImpl.c \
        $(CTARGDIR)FieldImpl.c \
        $(CTARGDIR)FrameID.c \
        $(CTARGDIR)MethodImpl.c \
        $(CTARGDIR)ObjectReferenceImpl.c \
        $(CTARGDIR)ReferenceTypeImpl.c \
        $(CTARGDIR)SDE.c \
        $(CTARGDIR)StackFrameImpl.c \
        $(CTARGDIR)StringReferenceImpl.c \
        $(CTARGDIR)ThreadGroupReferenceImpl.c \
        $(CTARGDIR)ThreadReferenceImpl.c \
        $(CTARGDIR)VirtualMachineImpl.c \
        $(CTARGDIR)bag.c \
        $(CTARGDIR)commonRef.c \
        $(CTARGDIR)classTrack.c \
        $(CTARGDIR)debugDispatch.c \
        $(CTARGDIR)debugInit.c \
        $(CTARGDIR)debugLoop.c \
        $(CTARGDIR)error_messages.c \
        $(CTARGDIR)eventFilter.c \
        $(CTARGDIR)eventHandler.c \
        $(CTARGDIR)eventHelper.c \
        $(CTARGDIR)inStream.c \
        $(CTARGDIR)invoker.c \
        $(CTARGDIR)log_messages.c \
        $(CTARGDIR)outStream.c \
        $(CTARGDIR)standardHandlers.c \
        $(CTARGDIR)stepControl.c \
        $(CTARGDIR)stream.c \
        $(CTARGDIR)threadControl.c \
        $(CTARGDIR)transport.c \
        $(CTARGDIR)util.c \
	$(CTARGDIR)linker_md.c \
	$(CTARGDIR)exec_md.c

#
# Rules.
#
include $(BUILDDIR)/common/Library.gmk

# We don't want to link against -ljava
JAVALIB=

#
# Add to ambient vpath so we pick up the library files
#
ifeq ($(PLATFORM), windows)
vpath %.c $(SHARE_SRC)/back:$(SHARE_SRC)/back
endif # PLATFORM
vpath %.c $(SHARE_SRC)/back:$(PLATFORM_SRC)/back

#
# Lint checking rules (we filter out the lint pass and only list errors)
#
linterrs: $(TEMPDIR)/linterrs
$(TEMPDIR)/linterrs: $(FILES_o)
	$(RM) $@
	$(MAKE) lint.clean
	$(MAKE) lint.errors > $@ 2>&1
	($(CAT) $@ | $(GREP) 'warning:' $(MY_LINT_FILTER) || exit 0)


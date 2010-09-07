#
# @(#)Makefile	1.35 10/03/23
# 
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
LIBRARY = hprof
PRODUCT = sun
LIBRARY_OUTPUT = hprof_jvmti

# Configure the CFLAGS for this library.

FILES_m = mapfile-vers

include $(BUILDDIR)/common/Defs.gmk

_OPT=$(CC_HIGHEST_OPT)

SRCDIR=$(SHARE_SRC)/demo/jvmti/hprof
PSRCDIR=$(PLATFORM_SRC)/demo/jvmti/hprof

# Use the mapfile-vers (See the mapfile located with this Makefile)
ifdef FILES_m
include $(BUILDDIR)/common/Mapfile-vers.gmk
endif

#
# Files to compile.
#
FILES_c = 				\
	$(CTARGDIR)debug_malloc.c 	\
	$(CTARGDIR)hprof_blocks.c 	\
	$(CTARGDIR)hprof_check.c 	\
	$(CTARGDIR)hprof_class.c 	\
	$(CTARGDIR)hprof_cpu.c 		\
	$(CTARGDIR)hprof_error.c 	\
	$(CTARGDIR)hprof_event.c 	\
	$(CTARGDIR)hprof_frame.c 	\
	$(CTARGDIR)hprof_init.c 	\
	$(CTARGDIR)hprof_io.c 		\
	$(CTARGDIR)hprof_ioname.c 	\
	$(CTARGDIR)hprof_listener.c 	\
	$(CTARGDIR)hprof_loader.c 	\
	$(CTARGDIR)hprof_md.c 		\
	$(CTARGDIR)hprof_monitor.c	\
	$(CTARGDIR)hprof_object.c	\
	$(CTARGDIR)hprof_reference.c	\
	$(CTARGDIR)hprof_site.c 	\
	$(CTARGDIR)hprof_stack.c 	\
	$(CTARGDIR)hprof_string.c 	\
	$(CTARGDIR)hprof_table.c 	\
	$(CTARGDIR)hprof_tag.c 		\
	$(CTARGDIR)hprof_tls.c 		\
	$(CTARGDIR)hprof_trace.c	\
	$(CTARGDIR)hprof_tracker.c 	\
	$(CTARGDIR)hprof_util.c

OTHER_INCLUDES = -I$(SRCDIR) \
	         -I$(SHARE_SRC)/npt \
	         -I$(PLATFORM_SRC)/npt \
	         -I$(SHARE_SRC)/demo/jvmti/java_crw_demo

ifeq ($(PLATFORM), windows)
  OTHER_LDLIBS += wsock32.lib winmm.lib
else
  LIBSOCKET = -lsocket
  OTHER_LDLIBS += $(LIBSOCKET) -lnsl -ldl
endif

#
# Tell library.gmk to copy the txt file first
#
INIT += $(LIBDIR)/jvm.hprof.txt

ifeq ($(PLATFORM), linux)
  # fork1() is declared in libjvm.so
  OTHER_LDLIBS += $(JVMLIB)
endif # PLATFORM

ifeq ($(PLATFORM), solaris)
  # Use of lint has been helpful, but lots of errors needed to be suppressed:
  OTHER_LINTFLAGS += -errtags=yes
  # We only care about errors in user include files
  OTHER_LINTFLAGS += -errhdr=%user
  OTHER_LINTFLAGS += -erroff=E_CAST_INT_TO_SMALL_INT
  # Logging gives lint fits, so we turn it off
  OTHER_LINTFLAGS += -DNDEBUG
  # We don't use the -Ncheck feature yet, it seems somewhat broken
  #OTHER_LINTFLAGS += -Ncheck -Nlevel=2
  # Even with above suppression, some files/code are hopelessly non-lintable
  #   socket.h has some old style C function prototypes in it (Solaris 9u2)
  MY_LINT_FILTER += | $(GREP) -v /usr/include
endif # PLATFORM

#
# This removes all asserts in the optimized version
#
CPPFLAGS_OPT += -DNDEBUG

#
# This puts logging code in both versions
#
CPPFLAGS_DBG += -DHPROF_LOGGING
CPPFLAGS_OPT += -DHPROF_LOGGING

#
# Library to compile.
#
include $(BUILDDIR)/common/Library.gmk

# We don't want to link against -ljava
JAVALIB=

#
# Add to ambient vpath so we pick up the library files
#
vpath %.c $(SRCDIR):$(PSRCDIR)

#
# Install the hprof prelude
#
$(LIBDIR)/jvm.hprof.txt: $(SRCDIR)/jvm.hprof.txt
	$(install-file)


#
# Lint checking rules (we filter out the lint pass and only list errors)
#
linterrs: $(TEMPDIR)/linterrs
$(TEMPDIR)/linterrs: $(FILES_o)
	$(prep-target)
	$(MAKE) lint.clean
	$(MAKE) lint.errors > $@ 2>&1
	($(CAT) $@ | $(GREP) 'warning:' $(MY_LINT_FILTER) || exit 0)

########################################################################
#

$(TEMPDIR)/extern_list.txt : $(FILES_o)
	$(prep-target)
	$(RM) $@.temp
	nm $(FILES_o) | fgrep -v UNDEF | fgrep GLOB | fgrep FUNC | cut -d'|' -f8 | sort | uniq > $@.temp
	$(MV) $@.temp $@

HTML_SRC_LIST=  $(SHARE_SRC)/classes/com/sun/demo/jvmti/hprof/Tracker.java \
		$(SHARE_SRC)/../*/demo/jvmti/hprof/hprof*.c \
		$(SHARE_SRC)/demo/jvmti/java_crw_demo/*.[ch]

HTML_INC_LIST=$(SHARE_SRC)/../*/demo/jvmti/hprof/hprof*.h

doc: $(TEMPDIR)/src_review.html

$(TEMPDIR)/src_review.html : create_html.ksh $(TEMPDIR)/extern_list.txt $(SRCDIR)/README.txt $(HTML_INC_LIST) $(HTML_SRC_LIST)
	$(prep-target)
	$(KSH) create_html.ksh $@.temp $(SRCDIR)/README.txt \
		"$(HTML_INC_LIST)"  "$(HTML_SRC_LIST)" $(TEMPDIR)/extern_list.txt
	$(MV) $@.temp $@

#--------------

everything: all linterrs test doc

clean clobber::
	$(RM) $(LIBDIR)/jvm.hprof.txt


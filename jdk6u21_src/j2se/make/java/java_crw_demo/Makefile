#
# @(#)Makefile	1.15 10/03/23
# 
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
LIBRARY = java_crw_demo
PRODUCT = sun
LIBRARY_OUTPUT = java_crw_demo

# Configure the CFLAGS for this library.

include $(BUILDDIR)/common/Defs.gmk

SRCDIR=$(SHARE_SRC)/demo/jvmti/java_crw_demo

# Use the mapfile-vers (See the mapfile located with this Makefile)
FILES_m = mapfile-vers
ifdef FILES_m
include $(BUILDDIR)/common/Mapfile-vers.gmk
endif

#
# Files to compile.
#
FILES_c = 				\
	$(CTARGDIR)java_crw_demo.c

OTHER_INCLUDES = -I$(SRCDIR)

ifeq ($(PLATFORM), linux)
  RUN_PREFIX=MALLOC_CHECK_=2
endif # PLATFORM

ifeq ($(PLATFORM), solaris)
  RUN_PREFIX=LD_PRELOAD=/usr/lib/watchmalloc.so.1 MALLOC_DEBUG=WATCH
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
  # Temporary, FIXUP before FCS
  CPPFLAGS_COMMON += -D_ALLOW_PRECRASH -D_ALLOW_PAUSE
endif # PLATFORM

#
# This removes all asserts in the optimized version
#
CPPFLAGS_OPT += -DNDEBUG

#
# Library to compile.
#
include $(BUILDDIR)/common/Library.gmk

ifneq ($(PLATFORM), windows)
  TEST_PROGRAM = $(OBJDIR)/test_crw
endif

build: $(TEST_PROGRAM)

# We don't want to link against -ljava
JAVALIB=

#
# Add to ambient vpath so we pick up the library files
#
vpath %.c $(SRCDIR)

#
# Lint checking rules (we filter out the lint pass and only list errors)
#
linterrs: $(TEMPDIR)/linterrs
$(TEMPDIR)/linterrs: $(FILES_o)
	$(RM) $@
	$(MAKE) lint.clean
	$(MAKE) lint.errors > $@ 2>&1
	($(CAT) $@ | $(GREP) 'warning:' $(MY_LINT_FILTER) || exit 0)

########################################################################
#
# Testing for java_crw_demo.c (Classfile Reader Writer)
#

CRW_ORIG_DIR=$(TEMPDIR)/test_classes/ORIG
CRW_CRWD_DIR=$(TEMPDIR)/test_classes/CRWD
RUN_CRW_PROGRAM=$(RUN_PREFIX) $(OBJDIR)/test_crw
CRW_OPTIONS=

CRW_TESTS = CrwTest1.java CrwTest2.java

# Need something to separate the *.class and *.dump files from each other
CLASSIMAGES =     $(CRW_TESTS:%.java=$(CRW_ORIG_DIR)/%.class)
CRW_CLASSIMAGES = $(CRW_TESTS:%.java=$(CRW_CRWD_DIR)/%.class)
CLASS_DUMPS =     $(CRW_TESTS:%.java=$(CRW_ORIG_DIR)/%.dump) \
	          $(CRW_TESTS:%.java=$(CRW_CRWD_DIR)/%.dump)
CLASS_DIFFS =     $(CRW_TESTS:%.java=$(CRW_CRWD_DIR)/%.diffs)

JAVAP=$(BOOTDIR)/bin/javap
JAVAP_FLAGS=-s -private -verbose

# Test rule for crw
test: all crw_sanity_test crw_injection_testing

# Create transformed classimages and dumps using javap
crw_dumps: $(CLASSIMAGES) $(CRW_CLASSIMAGES) $(CLASS_DUMPS)

# Test for no injection, but classimage re-writing
crw_sanity_test: crw_dumps $(CLASS_DIFFS)

# Test for bytecodes injection
crw_injection_testing: $(OBJDIR)/test_crw
	@$(MKDIR) -p $(CRW_CRWD_DIR)_c
	$(MAKE) CRW_CRWD_DIR=$(CRW_CRWD_DIR)_c CRW_OPTIONS=-c crw_dumps
	@$(MKDIR) -p $(CRW_CRWD_DIR)_r
	$(MAKE) CRW_CRWD_DIR=$(CRW_CRWD_DIR)_r CRW_OPTIONS=-r crw_dumps
	@$(MKDIR) -p $(CRW_CRWD_DIR)_n
	$(MAKE) CRW_CRWD_DIR=$(CRW_CRWD_DIR)_n CRW_OPTIONS=-n crw_dumps

# Build crw process
$(OBJDIR)/test_crw: $(OBJDIR)/java_crw_demo.o $(OBJDIR)/test_crw.o
	@$(RM) $@
	$(CC) $(CFLAGS) -g -o $@ $(OBJDIR)/java_crw_demo.o $(OBJDIR)/test_crw.o

# Clean up the crw testing files
clean clobber:: crw_clean
crw_clean:
	$(RM) -r $(TEMPDIR)/test_classes

# Rebuild all crw class images when crw changes
$(CRW_CLASSIMAGES): $(OBJDIR)/test_crw

# Create original classimage from java source
$(CRW_ORIG_DIR)/%.class: $(SRCDIR)/test_sources/%.java
	$(prep-target)
	$(BOOTDIR)/bin/javac -g -d $(@D) $<

# Run crw process and create crw classimage
$(CRW_CRWD_DIR)/%.class: $(CRW_ORIG_DIR)/%.class
	$(prep-target)
	@$(RM) $@.temp
	$(RUN_CRW_PROGRAM) $(CRW_OPTIONS) $< $@.temp
	$(MV) $@.temp $@

# Create a classimage dump
%.dump: %.class
	@$(RM) $@
	@$(RM) $@.temp
	$(JAVAP) $(JAVAP_FLAGS) -bootclasspath $(@D) `basename $@ .dump` > $@.temp 2>&1
	@if $(GREP) '^ERROR:' $@.temp ; then \
	    $(CAT) $@.temp ; \
	    exit 1; \
	fi
	@$(SED) -e 's@Stack=[0-9][0-9]*@Stack=NNN@' $@.temp > $@
	@$(RM) $@.temp

# Create a diff of two classimage dumps
$(CRW_CRWD_DIR)/%.diffs: $(CRW_ORIG_DIR)/%.dump
	@$(RM) $@
	$(DIFF) -w $< $(@D)/`basename $<`
	@$(TOUCH) $@

ROUGHDIR=/tmp/bciclasses

rough:
	$(RM) -r $(ROUGHDIR)
	@cd $(CLASSDESTDIR) ; \
	for i in `find . -name \*.class` ; do \
	    echo "Running javap on original file"; \
	    $(JAVAP) $(JAVAP_FLAGS) -bootclasspath . \
		`echo $$i | sed -e 's@^./@@' | sed -e 's@.class@@' | sed -e 's@/@.@g'` \
		> $$i.dump 2>  $$i.dumperr ; \
	    if [ -s $$i.dumperr ] ; then \
		echo "ERROR $$i.dumperr has size?"; \
	    fi; \
	    if [ ! -s $$i.dump ] ; then \
		echo "ERROR Missing $$i.dump, not created by $(JAVAP)?"; \
	    fi; \
	    $(RM) core core.*; \
	    mkdir -p $(ROUGHDIR)/`dirname $$i` ; \
	    echo "Running test_crw on original file"; \
	    echo "$(RUN_CRW_PROGRAM) -n -r -c $$i $(ROUGHDIR)/$$i" ; \
	    $(RUN_CRW_PROGRAM) -n -r -c $$i $(ROUGHDIR)/$$i ; \
	    if [ ! -s $(ROUGHDIR)/$$i ] ; then \
		echo "ERROR Missing $(ROUGHDIR)/$$i, not created by $(RUN_CRW_PROGRAM)?"; \
		exit 1; \
	    fi; \
	    if [ "`ls core core.* 2> /dev/null`" != "" ] ; then \
		echo "ERROR Detected core file"; \
		exit 1;  \
	    fi; \
	    echo "Running javap on processed file"; \
	    $(JAVAP) $(JAVAP_FLAGS) -bootclasspath $(ROUGHDIR) \
		`echo $$i | sed -e 's@^./@@' | sed -e 's@.class@@' | sed -e 's@/@.@g'` \
		> $(ROUGHDIR)/$$i.dump 2>  $(ROUGHDIR)/$$i.dumperr ; \
	    if [ -s $(ROUGHDIR)/$$i.dumperr ] ; then \
		echo "ERROR $(ROUGHDIR)/$$i.dumperr has size?"; \
	    fi; \
	    if [ ! -s $(ROUGHDIR)/$$i.dump ] ; then \
		echo "ERROR Missing $(ROUGHDIR)/$$i.dump, not created by $(JAVAP)?"; \
	    fi; \
	done

#--------------

everything: all linterrs test doc


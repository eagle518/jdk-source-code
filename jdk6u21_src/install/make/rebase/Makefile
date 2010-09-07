#  @(#)Makefile	1.15 10/04/15
#
# Copyright (c) 2006, 2010, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
#
# Makefile for Rebasing JRE and SDK jar files prior to building bundles
#

INSTALL_BUILDDIR=..
include $(INSTALL_BUILDDIR)/common/Defs.gmk

REBASE_TEMPDIR=$(TEMP_DIR)/rebase

all:: 
	@$(ECHO) $(ARCH) "Rebase  started:  " $(FULL_VERSION)

all build:: rebase-dlls check-dependents

# The following DLL's are not to be REBASEd
NOTREBASED = $(MS_RUNTIME_LIBRARIES) deployJava1.dll npdeployJava1.dll
NOTREBASED_find_filter = $(NOTREBASED:%=-name % -prune -o)

# Omit these signed dlls from the rebase operation: (signed images can not be rebased)
#    deployJava1.dll
#    npdeployJava1.dll
#    unicows.dll: 
#    msvcrt.dll
#
$(REBASE_TEMPDIR)/jre-rebase.input:
	$(RM) $@
	$(MKDIR) -p $(@D)
	$(FIND) $(JRE_IMAGE_DIR) $(NOTREBASED_find_filter) \
	    -type f '(' -name '*.$(LIBRARY_SUFFIX)' -o -name '*.ocx' ')' \
	    -print > $@

$(REBASE_TEMPDIR)/jdk-rebase.input:
	$(RM) $@
	$(MKDIR) -p $(@D)
	$(FIND) $(JDK_IMAGE_DIR) $(NOTREBASED_find_filter) \
	    -type f '(' -name '*.$(LIBRARY_SUFFIX)' -o -name '*.ocx' ')' \
	    -print > $@

rebase-dlls:: $(REBASE_TEMPDIR)/jre-rebase.input $(REBASE_TEMPDIR)/jdk-rebase.input
	$(REBASE) -b $(REBASE_ADDR) -R $(JRE_IMAGE_DIR) \
		  -G $(REBASE_TEMPDIR)/jre-rebase.input
	$(REBASE) -b $(REBASE_ADDR) -R $(JDK_IMAGE_DIR) \
		  -G $(REBASE_TEMPDIR)/jdk-rebase.input

$(REBASE_TEMPDIR)/check-dependents.input: $(REBASE_TEMPDIR)/jre-rebase.input $(REBASE_TEMPDIR)/jdk-rebase.input
	$(RM) $@
	$(MKDIR) -p $(@D)
	$(CAT) $< >> $@
	$(FIND) $(JDK_IMAGE_DIR) -type f -name '*.$(EXE_SUFFIX)' -print >> $@
	$(FIND) $(JRE_IMAGE_DIR) -type f -name '*.$(EXE_SUFFIX)' -print >> $@

UNWANTED_DEPENDENT=msvcp71.dll
DUMPBIN=$(LINK) /dump
DEPENDENTS=$(DUMPBIN) /dependents

check-dependents: $(REBASE_TEMPDIR)/check-dependents.input
	@$(ECHO) Checking for unwanted use of $(UNWANTED_DEPENDENT)
	@for i in `$(CAT) $<` ; do \
	    $(ECHO) "$(DEPENDENTS) $$i"; \
	    $(RM) $(REBASE_TEMPDIR)/dll_dump; \
	    $(ECHO) "$(DEPENDENTS) $$i" > $(REBASE_TEMPDIR)/dll_dump; \
	    $(DEPENDENTS) $$i >> $(REBASE_TEMPDIR)/dll_dump; \
	    if [ "`$(CAT) $(REBASE_TEMPDIR)/dll_dump | $(GREP) -i $(UNWANTED_DEPENDENT)`" != "" ] ; then \
                $(ECHO) File $$i contains a dependency on $(UNWANTED_DEPENDENT); \
                $(CAT) $(REBASE_TEMPDIR)/dll_dump; \
		$(CP) $$i $(REBASE_TEMPDIR); \
		exit 1; \
	    fi; \
	    $(RM) $(REBASE_TEMPDIR)/dll_dump; \
	done

clean:

clobber:


# this should be the last rule in this file:
all::
	@$(ECHO) $(ARCH) "Rebase finished:  " $(FULL_VERSION)
#
# Phonies to avoid accidents.
#
.PHONY: all rebase-dlls clean clobber

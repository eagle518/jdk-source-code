#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
# @(#)Makefile	1.6 03/12/19
#
#
# Makefile for building and packaging all of the JDK and the JRE. See
# also included files.
#
#
# Most frequently used targets:
#
#    all            -- build optimized and debug versions of the core JDK and 
#                      ext (Defined in this makefile only)
#    clobber        -- clobber the builds of the core JDK and ext (defeined
#                      in this makefile only)
#

INSTALL_BUILDDIR= ../../..
PRODUCT = stub
SRCDIR=../../../../src/windows/patcher-stub/

TARGET_NAME=stub-patcher
TARGET=$(TARGET_NAME)$(SUFFIX)$(EXE_SUFFIX)


TMPDIR=$(TEMP_DIR)/patcher/$(PRODUCT)

include $(INSTALL_BUILDDIR)/common/Defs.gmk

CPPFLAGS += -D FULL_VERSION="\"$(FULL_VERSION)\""
CPPFLAGS += -D RELEASE="\"$(RELEASE)\""

CPPFLAGS += -D WINDOWS -D _MBCS -D _ATL_STATIC_REGISTRY 
CPPFLAGS += $(CPPFLAGS_$(VARIANT))
CPPFLAGS +=  -c 

# Linker flags

LDFLAGS += version.lib shell32.lib


all:	$(OBJDIR) debug post-build


$(OBJDIR):
	$(MKDIR) -p $(OBJDIR)

$(BIN_BUNDLEDIR):
	$(MKDIR) -p  $(BIN_BUNDLEDIR)

FILES_c= \
	$(SRCDIR)main.cpp

FILES_o	= $(patsubst %.cpp, %.$(OBJECT_SUFFIX), $(addprefix $(OBJDIR)/, $(notdir $(FILES_c))))

$(OBJDIR)/%.$(OBJECT_SUFFIX): $(SRCDIR)%.cpp
	$(COMPILE.cc) $(CC_OBJECT_OUTPUT_FLAG)$(OBJDIR)/$*.$(OBJECT_SUFFIX) $<

$(OBJDIR)/$(TARGET): $(FILES_o)
	$(LINK) $(LDFLAGS) $(FILES_o)


build: $(OBJDIR) $(BIN_BUNDLEDIR) $(OBJDIR)/$(TARGET)

debug:
	$(MKDIR) -p $(OBJDIR)$(DBG_SUFFIX)
	$(MAKE) build VARIANT=DBG

optimized:
	$(MKDIR) -p $(OBJDIR)
	$(MAKE) build VARIANT=OPT


post-build: optimized $(BIN_BUNDLEDIR)
#	$(CP) $(OBJDIR)/$(TARGET) $(BIN_BUNDLEDIR)/$(FINAL_STUB_TARGET)


clean::
	$(RM) -r $(OBJDIR) $(OBJDIR)$(DBG_SUFFIX)
	$(RM) -r $(TMPDIR)
#	$(RM) -rf $(BIN_BUNDLEDIR)/$(FINAL_STUB_TARGET)

#
# Phonies to avoid accidents.
#
.PHONY: all optimized debug clobber clean build post-build









#
# @(#)Makefile	1.22 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building  Pack200
#

BUILDDIR = ../../../..
PACKAGE = com.sun.java.util.jar.pack
LIBRARY = unpack
PRODUCT = sun
PGRM = unpack200
include $(BUILDDIR)/common/Defs.gmk

CPLUSPLUSLIBRARY=true

FILES_m = mapfile-vers
include $(BUILDDIR)/common/Mapfile-vers.gmk

#
# Files to compile
#
AUTO_FILES_JAVA_DIRS = com/sun/java/util/jar/pack

include FILES_cpp.gmk

FILES_export = $(PKGDIR)/NativeUnpack.java

#
# Rules
include $(BUILDDIR)/common/Library.gmk

build: unpacker

vpath %.cpp $(SHARE_SRC)/native/$(PKGDIR)





ifeq ($(STANDALONE),true)
  ZIPOBJDIR = $(OUTPUTDIR)/tmp/sun/java.util.zip/zip/$(OBJDIRNAME)

  ZIPOBJS =  $(ZIPOBJDIR)/zcrc32.$(OBJECT_SUFFIX) \
	     $(ZIPOBJDIR)/deflate.$(OBJECT_SUFFIX) \
	     $(ZIPOBJDIR)/trees.$(OBJECT_SUFFIX) \
	     $(ZIPOBJDIR)/zadler32.$(OBJECT_SUFFIX) \
	     $(ZIPOBJDIR)/zutil.$(OBJECT_SUFFIX) \
	     $(ZIPOBJDIR)/inflate.$(OBJECT_SUFFIX) \
	     $(ZIPOBJDIR)/infblock.$(OBJECT_SUFFIX) \
	     $(ZIPOBJDIR)/infcodes.$(OBJECT_SUFFIX) \
	     $(ZIPOBJDIR)/inftrees.$(OBJECT_SUFFIX) \
	     $(ZIPOBJDIR)/infutil.$(OBJECT_SUFFIX) \
	     $(ZIPOBJDIR)/inffast.$(OBJECT_SUFFIX)

  ZINCLUDE=-I$(SHARE_SRC)/native/java/util/zip/zlib-1.1.3
  OTHER_CXXFLAGS += $(ZINCLUDE)
  LDDFLAGS += $(ZIPOBJS)
else
  OTHER_CXXFLAGS += -DNO_ZLIB -DUNPACK_JNI
  OTHER_LDLIBS += $(JVMLIB)
endif

CXXFLAGS_DBG += -DFULL
CXXFLAGS_OPT += -DPRODUCT
CXXFLAGS_COMMON += -DFULL

ifeq ($(PLATFORM), windows) 
  CXXFLAGS_COMMON += $(MS_RUNTIME_OPTION)
  LDOUTPUT = -Fe

  RC_FLAGS += /D "J2SE_FNAME=$(PGRM).exe" \
	/D "J2SE_INTERNAL_NAME=$(PGRM)" \
	/D "J2SE_FTYPE=0x1L" \
	-D "MANIFEST=$(PGRM).exe.manifest" \
        -i $(OBJDIR) 

  RES = $(OBJDIR)/$(PGRM).res

  # Files built here do not compile with warning level 3 if warnings are fatal
  COMPILER_WARNINGS_FATAL=false
else
  LDOUTPUT = -o #Have a space 
  LDDFLAGS += -lc
  OTHER_LDLIBS  += $(LIBCXX)
# setup the list of libraries to link in...
ifeq ($(PLATFORM), linux)
ifeq ("$(CC_VER_MAJOR)", "3")
  OTHER_LDLIBS  += -Wl,-Bstatic -lgcc_eh -Wl,-Bdynamic
endif
endif #LINUX
endif #PLATFORM

UNPACK_EXE = $(BINDIR)/unpack200$(EXE_SUFFIX)

UNPACK_EXE_FILES_cpp =  $(filter-out $(CTARGDIR)jni.cpp,$(FILES_cpp))

UNPACK_EXE_FILES_cpp += $(CTARGDIR)main.cpp

UNPACK_EXE_FILES_o = $(addprefix $(COBJDIR)/, \
		     	$(notdir  \
		     	$(patsubst %.cpp,%.$(OBJECT_SUFFIX),$(UNPACK_EXE_FILES_cpp)) \
		     ))

COBJDIR = $(strip $(subst unpack,unpack-cmd,$(OBJDIR)))

all: build

build: prop launcher

prop:
	$(MAKE) -C prop

launcher:
	$(MAKE) -C launcher

unpacker:
	$(MAKE) $(UNPACK_EXE) STANDALONE=true LDMAPFLAGS_OPT= LDMAPFLAGS_DBG=

ifeq ($(PLATFORM), windows)
IMVERSIONVALUE=$(JDK_MINOR_VERSION).$(JDK_MICRO_VERSION).$(J2SE_UPDATE_VER).$(COOKED_BUILD_NUMBER)
SED_ALL_MANIFEST=$(SED) -e 's%IMVERSION%$(IMVERSIONVALUE)%g;s%PROGRAM%$(PGRM)%g'
manifestdummy1 := $(shell $(MKDIR) -p $(OBJDIR))
manifestdummy2 := $(shell $(SED_ALL_MANIFEST) $(J2SE_TOPDIR)/src/windows/resource/template.manifest > $(OBJDIR)/$(PGRM).exe.manifest)
winres::  $(RES)
else
winres::
	$(ECHO) "Resource files not required for Unix"
endif

$(UNPACK_EXE): $(UNPACK_EXE_FILES_o) winres 
	$(prep-target)
	$(LINKER)  $(LDDFLAGS) $(UNPACK_EXE_FILES_o) $(RES) $(LIBCXX) $(LDOUTPUT)$(TEMPDIR)/unpack200$(EXE_SUFFIX)
	$(CP) $(TEMPDIR)/unpack200$(EXE_SUFFIX) $(UNPACK_EXE)


ifeq ($(PLATFORM), windows) 
$(RES):: $(VERSIONINFO_RESOURCE)
	$(prep-target)
	$(RC) $(RC_FLAGS) $(CC_OBJECT_OUTPUT_FLAG)$(@) $(VERSIONINFO_RESOURCE)
endif

$(COBJDIR)/%.$(OBJECT_SUFFIX): $(CTARGDIR)%.cpp
	$(prep-target)
	$(COMPILE.cc) $(CC_OBJECT_OUTPUT_FLAG)$(COBJDIR)/$*.$(OBJECT_SUFFIX) $<

#
# Clean up.
#
clean::
	$(RM) -r $(CLASSDESTDIR)/com/sun/java/util/jar/pack $(TEMPDIR)
	$(RM) -r $(OUTPUTDIR)/tmp/$(PRODUCT)/$(PACKAGE)
	$(MAKE) -C prop clean
	$(MAKE) -C launcher clean


.PHONY: unpacker clean winres prop launcher

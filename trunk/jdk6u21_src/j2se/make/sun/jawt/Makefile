#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.47 10/03/23
#

BUILDDIR = ../..
PACKAGE = sun.awt
LIBRARY = jawt
PRODUCT = sun

include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
ifeq ($(PLATFORM), windows)
FILES_cpp = $(TARGDIR)jawt.cpp
else # PLATFORM
FILES_c = $(TARGDIR)jawt.c
endif # PLATFORM

FILES_h = $(INCLUDEDIR)/jawt.h \
    $(PLATFORM_INCLUDE)/jawt_md.h

#
# Use mapfile
#
FILES_m = mapfile-vers
include $(BUILDDIR)/common/Mapfile-vers.gmk


ifeq ($(PLATFORM), windows)
#
# Extra C flags.
#

OTHER_CXXFLAGS += $(GX_OPTION) -DUNICODE -D_UNICODE

#
# Other extra flags needed for compiling.
#
CPPFLAGS += -I$(SHARE_SRC)/native/common \
	-I$(DXSDK_INCLUDE_PATH) \
	-I$(PLATFORM_SRC)/native/sun/windows \
	-I$(CLASSHDRDIR)/../../awt/CClassHeaders \
        -I$(SHARE_SRC)/native/sun/awt/debug \
        -I$(SHARE_SRC)/native/sun/font \
        -I$(SHARE_SRC)/native/sun/java2d \
        -I$(SHARE_SRC)/native/sun/java2d/loops \
        -I$(SHARE_SRC)/native/sun/java2d/pipe \
	-I$(PLATFORM_SRC)/native/sun/java2d \
	-I$(PLATFORM_SRC)/native/sun/java2d/windows \
	-I$(PLATFORM_SRC)/native/sun/java2d/d3d \
	-I$(SHARE_SRC)/native/$(PKGDIR)/image/cvutils \
	-I$(SHARE_SRC)/javavm/export \
	-I$(PLATFORM_SRC)/javavm/export
INSTALL_DOT_LIB = true

vpath %.cpp   $(PLATFORM_SRC)/native/sun/windows

#
# Libraries to link in.
#
ifeq ($(ARCH_DATA_MODEL), 64)
OTHER_LDLIBS = $(OBJDIR)/../../awt/$(OBJDIRNAME)/awt.lib 
else
OTHER_LDLIBS = /nod:kernel32.lib /nod:advapi32.lib /nod:user32.lib \
               /nod:gdi32.lib /nod:shell32.lib /nod:comdlg32.lib \
               /nod:version.lib /nod:mpr.lib /nod:rasapi32.lib \
               /nod:winmm.lib /nod:winspool.lib /nod:vfw32.lib \
               /nod:secur32.lib /nod:oleacc.lib /nod:oledlg.lib \
               /nod:sensapi.lib \
               $(UNICOWS_LIB_PATH)/unicows.lib kernel32.lib \
	       $(OBJDIR)/../../awt/$(OBJDIRNAME)/awt.lib 
endif
else # PLATFORM

#
# Other extra flags needed for compiling.
#
CPPFLAGS += -I$(OPENWIN_HOME)/include \
	    -I$(MOTIF_DIR)/include \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/debug \
            -I$(SHARE_SRC)/native/$(PKGDIR)/image \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/image/cvutils \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/alphacomposite \
            -I$(SHARE_SRC)/native/$(PKGDIR)/medialib \
            -I$(PLATFORM_SRC)/native/$(PKGDIR)/medialib \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/../java2d/loops \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/../java2d/pipe \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/../java2d/opengl \
	    -I$(PLATFORM_SRC)/native/$(PKGDIR)/../java2d/opengl \
	    -I$(PLATFORM_SRC)/native/$(PKGDIR)/../java2d/x11 \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/../dc/doe \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/../dc/path \
            -I$(PLATFORM_SRC)/native/$(PKGDIR)/../jdga \
	    $(EVENT_MODEL)
#
# Libraries to link in.
#
ifeq ($(PLATFORM), solaris)
OTHER_LDLIBS = -L$(LIBDIR)/$(LIBARCH) -L$(OPENWIN_LIB) -L$(LIBDIR)/$(LIBARCH)/xawt -lmawt
endif # PLATFORM

ifeq ($(PLATFORM), linux)
OTHER_LDLIBS = -L$(LIBDIR)/$(LIBARCH) -lawt -L$(LIBDIR)/$(LIBARCH)/xawt -lmawt
endif # PLATFORM

endif # PLATFORM


#
# Lets not remove any class files.
#
DONT_CLOBBER_CLASSES = true

#
# Rules
#
include $(BUILDDIR)/common/Library.gmk

build: $(FILES_h)

ifeq ($(PLATFORM), windows)
include make.depend
endif

#
# Install header files.
#
$(INCLUDEDIR)/%.h: $(SHARE_SRC)/javavm/export/%.h
	$(install-file)
$(PLATFORM_INCLUDE)/%.h: $(PLATFORM_SRC)/javavm/export/%.h
	$(install-file)


#
# Extra clean rule.
#
clean clobber::
	$(RM) $(FILES_h)


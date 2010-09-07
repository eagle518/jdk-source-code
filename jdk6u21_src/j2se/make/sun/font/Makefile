#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.85 10/03/23
#
# Makefile for building all of font tools
#

BUILDDIR = ../..
PACKAGE = sun.font
LIBRARY = fontmanager
PRODUCT = sun

# Indicate we want the C++ compiler to do the linking.
CPLUSPLUSLIBRARY=true

include $(BUILDDIR)/common/Defs.gmk

#
# Files
#

#
# Use higher optimization level
#
_OPT = $(CC_HIGHER_OPT)

include FILES_c.gmk

AUTO_FILES_JAVA_DIRS = sun/font
FILES_java += java/awt/Font.java java/text/Bidi.java 

FILES_export = \
	java/awt/Font.java \
	java/text/Bidi.java \
	sun/font/FileFont.java \
	sun/font/FileFontStrike.java \
	sun/font/FontManager.java \
	sun/font/GlyphList.java \
	sun/font/NativeFont.java \
	sun/font/StrikeCache.java \
	sun/font/Type1Font.java \
	sun/font/TrueTypeFont.java \
	sun/java2d/loops/DrawGlyphList.java \
	sun/java2d/loops/DrawGlyphListAA.java \
	sun/java2d/loops/DrawGlyphListLCD.java \
	sun/java2d/loops/GraphicsPrimitive.java

ifeq ($(PLATFORM), windows)

  # Files built here do not compile with warning level 3 if warnings are fatal
  COMPILER_WARNINGS_FATAL=false

  LDLIBS += user32.lib gdi32.lib $(OBJDIR)/../../../sun.awt/awt/$(OBJDIRNAME)/awt.lib

  ifeq ($(BCHECKER), 1)
    CC = C:/BChecker/nmcl.exe
  endif

  OTHER_CFLAGS += -DCC_NOEX
  OTHER_CXXFLAGS += -DCC_NOEX

else # PLATFORM

  FILES_export += \
	sun/font/NativeStrike.java \
	sun/font/NativeStrikeDisposer.java \
	sun/font/X11TextRenderer.java \
	sun/awt/X11GraphicsEnvironment.java

endif # PLATFORM

#ifeq ($(PLATFORM), solaris)
#  FILES_reorder += reorder-$(ARCH)
#endif
include $(BUILDDIR)/common/Mapfile-vers.gmk
include $(BUILDDIR)/common/Library.gmk


#
# Add to the ambient vpath to pick up files in subdirectories
#
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/t2k
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/t2k/ttHints
vpath %.c   $(PLATFORM_SRC)/native/$(PKGDIR)
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/bidi
vpath %.cpp   $(SHARE_SRC)/native/$(PKGDIR)/layout
vpath %.cpp   $(SHARE_SRC)/native/$(PKGDIR)/opentype
vpath %.cpp   $(SHARE_SRC)/native/$(PKGDIR)/layoutengine
vpath %.cpp   $(SHARE_SRC)/native/$(PKGDIR)

#
# Fontmanager is headless (independent of X/Motif)
#
CFLAGS       += -DHEADLESS
CXXFLAGS     += -DHEADLESS

ifeq ($(PLATFORM), windows)
vpath %.cpp   $(PLATFORM_SRC)/native/sun/windows
else # PLATFORM

#
# Libraries to link, and other C flags.
#

ifeq ($(PLATFORM), linux)
  CXXFLAGS += -DNEED_BOGUS_DELETE_OPERATORS
else
  # Note that on Solaris, fontmanager is built against the headless library.
  LDFLAGS      += -L$(LIBDIR)/$(LIBARCH)/headless
endif

# setup the list of libraries to link in...
ifeq ($(PLATFORM), linux)
 OTHER_LDLIBS  += -lawt $(LIBM) $(LIBCXX)
  ifeq ("$(CC_VER_MAJOR)", "3")
    OTHER_LDLIBS  += -Wl,-Bstatic -lgcc_eh -Wl,-Bdynamic
  endif                           
else
  OTHER_LDLIBS  += -lawt -L$(LIBDIR)/$(LIBARCH)/xawt -lmawt $(LIBM) $(LIBCXX)
endif

endif # PLATFORM

# set up compile flags..

CPPFLAGS += -I$(SHARE_SRC)/native/$(PKGDIR) \
            -I$(SHARE_SRC)/native/$(PKGDIR)/t2k \
            -I$(SHARE_SRC)/native/$(PKGDIR)/layout \
            -I$(SHARE_SRC)/native/$(PKGDIR)/opentype \
            -I$(SHARE_SRC)/native/$(PKGDIR)/layoutengine \
            -I$(SHARE_SRC)/native/$(PKGDIR)/bidi \
	    -I$(SHARE_SRC)/native/sun/awt/image/cvutils \
	    -I$(PLATFORM_SRC)/native/sun/awt \
	    -I$(SHARE_SRC)/native/sun/awt/debug \
	    -I$(SHARE_SRC)/native/sun/java2d/loops \
	    -I$(SHARE_SRC)/native/sun/java2d/pipe \
            -I$(PLATFORM_SRC)/native/sun/java2d \
	    -I$(SHARE_SRC)/native/sun/java2d \
	    -I$(TEMPDIR)/../../sun.awt/awt/CClassHeaders

ifeq ($(PLATFORM), windows)
CPPFLAGS += -I$(PLATFORM_SRC)/native/sun/windows
endif
#
# Has been converted to the JNI: generate JNI-style header files
#
JAVAHFLAGS += -jni

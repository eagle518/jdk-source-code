#
# @(#)Makefile	1.363 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
PACKAGE = sun.awt
LIBRARY = awt
PRODUCT = sun

#
# Tell Defs.gmk that VIS is needed
#
VIS_NEEDED=true

include $(BUILDDIR)/common/Defs.gmk

#
# Use highest optimization level
#
_OPT = $(CC_HIGHEST_OPT)

OTHER_CFLAGS += -D__MEDIALIB_OLD_NAMES -D__USE_J2D_NAMES

#
# Files
#

#
# Include all Java source files in sun/awt and sun/java2d, except for the
# sun/awt/resources directory, which is handled by java/awt/Makefile.
#
AUTO_FILES_JAVA_DIRS = sun/awt sun/java2d com/sun/awt
AUTO_JAVA_PRUNE = resources

ifeq ($(PLATFORM), windows)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv WINDOWS

#
# Files
#
include FILES_c_windows.gmk
include FILES_export_windows.gmk

#
# Prune files that are currently unused on Windows (but still used on
# Solaris/Linux).
#
AUTO_JAVA_PRUNE += RemoteOffScreenImage.java

OTHER_CFLAGS += $(GX_OPTION) -DMLIB_NO_LIBSUNMATH -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x400
OTHER_CXXFLAGS += $(GX_OPTION) -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x400

ifeq ($(ARCH_DATA_MODEL), 64)
  OTHER_CFLAGS += -DMLIB_OS64BIT
endif

# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ WINDOWS
endif # PLATFORM

ifneq ($(PLATFORM), windows)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv SOLARIS/LINUX

#
# Files
#
include FILES_c_unix.gmk
include FILES_export_unix.gmk
#
# Prune the sun/awt/X11 subdirectory, which is handled by sun/xawt/Makefile.
# Also prune files that are currently unused on Solaris/Linux (but still
# used on Windows).
#
AUTO_JAVA_PRUNE += X11 AWTCharset.java

ifeq  ($(PLATFORM), solaris)
FILES_c = $(FILES_2D_c)
FILES_c += awt_LoadLibrary.c
OTHER_LDLIBS = $(JVMLIB) $(LIBM) -ldl
ifeq ($(CC_VER), 5.8)
  ifndef REMOVE_ALL_WORKAROUNDS
    ifeq ($(ARCH_FAMILY), i586)
      # If on Solaris 11 (or dlfcn.h has unknown_control_flow pragmas in it)
      #   we trigger a SS11 bug that causes the compiler to crash (bug 6343678)
      #   Here we check to see if we need this workaround and change the opt 
      #   settings for this one file.
      USE_WORKAROUND:=$(shell $(GREP) unknown_control_flow /usr/include/dlfcn.h)
      ifneq ($(USE_WORKAROUND),)
	ifeq ($(FASTDEBUG), true)
          CFLAGS_DBG/awt_Mlib.o = -xO0 \
	    $(warning  "WARNING: Using workaround for SS11 bug 6343678, on $@")
	endif
        # Compiler bug 6343678 need to prevent a SEGV in the compiler
        CFLAGS_OPT/awt_Mlib.o = -g \
	  $(warning  "WARNING: Using workaround for SS11 bug 6343678, on $@")
      endif
    endif
  endif
endif
endif

ifeq  ($(PLATFORM), linux)
FILES_c = $(FILES_2D_c)
FILES_c += awt_LoadLibrary.c
OTHER_CFLAGS += -DMLIB_NO_LIBSUNMATH
OTHER_CFLAGS += -DMOTIF_VERSION=2
OTHER_LDLIBS = $(JVMLIB) $(LIBM) -ldl
endif

FILES_c += initIDs.c

# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ SOLARIS/LINUX
endif # PLATFORM 

# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv SOLARIS-SPARC
# solaris-sparc and solaris-sparcv9 both build 'vis'
ifeq ($(ARCH_FAMILY), sparc)
  FILES_c += $(FILES_2D_vis)
  ASFLAGS += -P
  FILES_s += $(TARGDIR)mlib_v_ImageCopy_blk.s
  INLINE_VIS = $(PLATFORM_SRC)/native/sun/awt/medialib/vis_$(ARCH_DATA_MODEL).il
  CFLAGS_sparcv9 = -DMLIB_OS64BIT
  
  CFLAGS += $(CFLAGS_$(ARCH)) -DMLIB_ADD_SUFF $(INLINE_VIS) \
	    -I$(SHARE_SRC)/native/sun/awt/medialib \
	    -I$(PLATFORM_SRC)/native/sun/awt/medialib \
	    -I$(PLATFORM_SRC)/native/sun/java2d/loops

vpath %.c  $(PLATFORM_SRC)/native/sun/java2d/loops
vpath %.c  $(SHARE_SRC)/native/sun/awt/medialib
vpath %.c  $(PLATFORM_SRC)/native/sun/awt/medialib
vpath %.s  $(PLATFORM_SRC)/native/sun/awt/medialib

else
  FILES_c += $(TARGDIR)MapAccelFunc.c
endif
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ SOLARIS-SPARC

#
# Resources
#

LOCALE_SET_DEFINITION = j2re

RESOURCE_BUNDLES_COMPILED_PROPERTIES = 
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes

ifeq ($(PLATFORM), windows)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv WINDOWS

RESOURCE_BUNDLES_COMPILED_PROPERTIES += sun/awt/windows/awtLocalization.properties
RESOURCE_SRC_DIR_2 = $(PLATFORM_SRC)/classes

# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ WINDOWS
endif # PLATFORM

include $(BUILDDIR)/common/Resources.gmk

ifeq ($(PLATFORM), linux)
  FILES_m = mapfile-vers-linux
  # libawt.so on Linux is statically linked with Motif and contains all the
  # Xm symbols. Mapfile is disabled so the symbols will remain public.
  LDNOMAP=true
endif

include $(BUILDDIR)/common/Mapfile-vers.gmk
include $(BUILDDIR)/common/Library.gmk

build: fontconfigs


ifeq ($(PLATFORM), windows)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv WINDOWS
#
# Additional dependencies.
#
# If make.depend is not working correctly, then it can be omitted
# if you always build from a clean workspace. 

include make.depend

# required additional rules:

# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ WINDOWS
endif # PLATFORM

ifneq ($(PLATFORM), windows)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv SOLARIS/LINUX

build: dgalibs

# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ SOLARIS/LINUX
endif # PLATFORM

#
# Rules to build the DebugHelper class
#
DEBUG_HELPER_JAVA = $(GENSRCDIR)/sun/awt/DebugHelper.java
DEBUG_HELPER_JAVA_M4 = $(SHARE_SRC)/classes/sun/awt/DebugHelper.java.m4
DEBUG_HELPER_CLASS = sun/awt/DebugHelper.class

ifdef SUN_AWT_DEBUG_CLASSES
  M4DEFS=-D SUN_AWT_DEBUG_CLASSES=$(SUN_AWT_DEBUG_CLASSES)
  DEBUG_BUILD_MSG = "** DebugHelper being built for DEBUG mode **"
else # SUN_AWT_DEBUG_CLASSES
  DEBUG_BUILD_MSG = "** DebugHelper being built for RELEASE mode **"
endif # SUN_AWT_DEBUG_CLASSES

# Preprocess thru m4 to produce legal .java file,
# force the creation of the .java source, and add the .java file
# to the list of the java compile files.
$(CLASSBINDIR)/$(DEBUG_HELPER_CLASS) : $(CLASSDESTDIR) $(DEBUG_HELPER_JAVA)
	$(ECHO) $(DEBUG_HELPER_JAVA) >> $(TEMPDIR)/.classes.list

$(DEBUG_HELPER_JAVA) : $(DEBUG_HELPER_JAVA_M4) $(GENSRCDIR)
	$(MKDIR) -p $(@D)
	@$(ECHO) $(DEBUG_BUILD_MSG)
	$(M4) $(M4DEFS) $(DEBUG_HELPER_JAVA_M4) > $(DEBUG_HELPER_JAVA)

debughelper : $(DEBUG_HELPER_JAVA) 

debughelper.clean :
	$(RM) $(DEBUG_HELPER_JAVA)
	$(RM) $(DEBUG_HELPER_CLASS)

clobber clean:: debughelper.clean

clobber clean:: java2d.clean motif.clean fontconfigs.clean \
        dgalib.clean 

java2d.clean:
	$(RM) -r $(CLASSBINDIR)/sun/java2d .classes.*

#
# Add to the ambient vpath to pick up files in subdirectories
#
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/image
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/image/gif
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/image/cvutils
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/shell
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/medialib
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/debug
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/utility
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/../java2d
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/../java2d/loops
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/../java2d/pipe
vpath %.cpp $(SHARE_SRC)/native/$(PKGDIR)/image
vpath %.c   $(PLATFORM_SRC)/native/$(PKGDIR)/robot_child

ifeq ($(PLATFORM), windows)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv WINDOWS
vpath %.cpp   $(PLATFORM_SRC)/native/sun/windows
vpath %.cpp   $(PLATFORM_SRC)/native/$(PKGDIR)
vpath %.cpp   $(PLATFORM_SRC)/native/$(PKGDIR)/../java2d/windows
vpath %.cpp   $(PLATFORM_SRC)/native/$(PKGDIR)/../java2d/d3d
vpath %.c     $(PLATFORM_SRC)/native/$(PKGDIR)/../java2d/opengl
vpath %.c     $(SHARE_SRC)/native/$(PKGDIR)/../java2d/opengl
vpath %.c     $(SHARE_SRC)/native/$(PKGDIR)/../font
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ WINDOWS
endif # PLATFORM

ifeq ($(PLATFORM), linux)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv LINUX
vpath %.c   $(PLATFORM_SRC)/native/$(PKGDIR)/robot_child
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/../java2d/opengl
vpath %.c   $(PLATFORM_SRC)/native/$(PKGDIR)/../java2d/opengl
vpath %.c   $(PLATFORM_SRC)/native/$(PKGDIR)/../java2d/x11
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/../font
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ LINUX
endif # PLATFORM

ifeq ($(PLATFORM), windows)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv WINDOWS
ifeq ($(ARCH_DATA_MODEL), 64)
OTHER_LDLIBS = kernel32.lib user32.lib gdi32.lib winspool.lib \
               imm32.lib ole32.lib uuid.lib $(JVMLIB) \
               shell32.lib comctl32.lib
else
OTHER_LDLIBS = /nod:kernel32.lib /nod:advapi32.lib /nod:user32.lib \
               /nod:gdi32.lib /nod:shell32.lib /nod:comctl32.lib /nod:comdlg32.lib \
               /nod:version.lib /nod:mpr.lib /nod:rasapi32.lib \
               /nod:winmm.lib /nod:winspool.lib /nod:vfw32.lib \
               /nod:secur32.lib /nod:oleacc.lib /nod:oledlg.lib \
               /nod:sensapi.lib $(UNICOWS_LIB_PATH)/unicows.lib \
               kernel32.lib user32.lib gdi32.lib winspool.lib \
               imm32.lib ole32.lib uuid.lib $(JVMLIB) \
               shell32.lib comctl32.lib
endif

clean:: awt.clean

awt.clean: 
	$(RM) $(TEMPDIR)/awt.ico 
	$(RM) $(TEMPDIR)/awt.res 
	$(RM) $(TEMPDIR)/hand.cur 
	$(RM) $(TEMPDIR)/check.bmp
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ WINDOWS
endif # PLATFORM

ifneq ($(PLATFORM), windows)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv SOLARIS/LINUX

ifeq ($(STATIC_MOTIF),true)
  LIBXM = $(MOTIF_LIB)/libXm.a
else # STATIC_MOTIF
  LIBXM = -lXm
endif # STATIC_MOTIF

LIBXTST = -lXtst

# Use -lXmu for EditRes support
LIBXMU_DBG	= -lXmu
LIBXMU_OPT	=
LIBXMU	        = $(LIBXMU_$(VARIANT))

#
# Extra C flags.
#
INIT += motif

motif: $(OBJDIR)/.library.dirs.$(ARCH) $(OBJDIR)/.library.motif.$(ARCH)

$(OBJDIR)/.library.motif.$(ARCH): $(MOTIF_LIB)/libXm.so.3
	$(CP) $(MOTIF_LIB)/libXm.so.3 $(LIBDIR)/$(LIBARCH)/$(TSOBJDIR)
	$(CD) $(LIBDIR)/$(LIBARCH)/$(TSOBJDIR); $(RM) libXm.so; \
		$(LN) -s libXm.so.3 libXm.so
	@-$(TOUCH) $(OBJDIR)/.library.motif.$(ARCH)

clean:: motif.clean

motif.clean :
	$(RM) $(LIBDIR)/$(LIBARCH)/$(TSOBJDIR)/libXm.*

#
# Solaris X11 Direct Graphics Access library
#

_DGALIBS_sparc = \
	libxinerama.so \
	libjdgaSUNWcg6.so \
	libjdgaSUNWffb.so \
	libjdgaSUNWm64.so \
	libjdgaSUNWafb.so

_DGALIBS_sparcv9 = \
	libxinerama.so \
	libjdgaSUNWcg6.so \
	libjdgaSUNWffb.so \
	libjdgaSUNWm64.so \
	libjdgaSUNWafb.so

_DGALIBS_i586 = 	# no i586 library yet

_DGALIBS_amd64 = 	# no amd64 library yet

DGALIBS = $(_DGALIBS_$(ARCH):%=$(LIBDIR)/$(LIBARCH)/%)

dgalibs: $(DGALIBS)

$(LIBDIR)/$(LIBARCH)/libxinerama.so: $(PLATFORM_SRC)/lib/$(ARCH)/libxinerama.so
	$(install-file)
	$(CHMOD) a+x $@

$(LIBDIR)/$(LIBARCH)/libjdgaSUNW%.so: $(PLATFORM_SRC)/lib/$(ARCH)/libjdgaSUNW%.so
	$(install-file)
	$(CHMOD) a+x $@

$(LIBDIR)/$(LIBARCH)/libjdgaSUNWafb.so: $(LIBDIR)/$(LIBARCH)/libjdgaSUNWffb.so
	$(prep-target)
	$(LN) -s libjdgaSUNWffb.so $(LIBDIR)/$(LIBARCH)/libjdgaSUNWafb.so

clean:: dgalib.clean

dgalib.clean:
	$(RM) $(LIBDIR)/$(LIBARCH)/libjdgaSUNW* $(LIBDIR)/$(LIBARCH)/libxinerama.so

# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ SOLARIS/LINUX
endif # PLATFORM

#
# Font configs
#

ifeq ($(PLATFORM), windows)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv WINDOWS

FONTCONFIGS_SRC = $(PLATFORM_SRC)/classes/sun/awt/windows
_FONTCONFIGS = \
        fontconfig.properties \
        fontconfig.98.properties 

FONTCONFIGS_SRC_PREFIX =

# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ WINDOWS
endif # PLATFORM

ifeq ($(PLATFORM), linux)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv LINUX

FONTCONFIGS_SRC	= $(PLATFORM_SRC)/classes/sun/awt/motif
_FONTCONFIGS	= \
	fontconfig.properties				\
	fontconfig.RedHat.properties			\
	fontconfig.RedHat.2.1.properties		\
	fontconfig.RedHat.3.properties		\
	fontconfig.RedHat.4.properties		\
	fontconfig.Sun.properties			\
	fontconfig.Turbo.properties			\
	fontconfig.SuSE.properties			\
	fontconfig.Ubuntu.properties

FONTCONFIGS_SRC_PREFIX = $(PLATFORM).

# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ LINUX
endif # PLATFORM

ifeq ($(PLATFORM), solaris)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv SOLARIS

FONTCONFIGS_SRC	= $(PLATFORM_SRC)/classes/sun/awt/motif
_FONTCONFIGS	= \
	fontconfig.OpenSolaris.properties	\
	fontconfig.properties	\
	fontconfig.5.9.properties	\
	fontconfig.5.8.properties

FONTCONFIGS_SRC_PREFIX = $(PLATFORM).

# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ SOLARIS
endif # PLATFORM

FONTCONFIGS     = $(_FONTCONFIGS:%=$(LIBDIR)/%.src)
BINARYFONTCONFIGS = $(_FONTCONFIGS:%.properties=$(LIBDIR)/%.bfc)

fontconfigs: $(FONTCONFIGS) $(BINARYFONTCONFIGS)

$(LIBDIR)/%.src: $(FONTCONFIGS_SRC)/$(FONTCONFIGS_SRC_PREFIX)%
	$(install-file)

$(LIBDIR)/%.bfc: $(FONTCONFIGS_SRC)/$(FONTCONFIGS_SRC_PREFIX)%.properties
	$(prep-target)
	$(RM) -r $(TEMPDIR)/CompileFontConfig
	$(MKDIR) -p $(TEMPDIR)/CompileFontConfig
	$(JAVAC) -d $(TEMPDIR)/CompileFontConfig $(BUILDDIR)/tools/CompileFontConfig/CompileFontConfig.java
	$(JAVA) -classpath $(TEMPDIR)/CompileFontConfig CompileFontConfig $< $(@)
	$(CHMOD) 444 $(@)
	@$(java-vm-cleanup)

fontconfigs.clean :
	$(RM) $(FONTCONFIGS)
	$(RM) $(BINARYFONTCONFIGS)
	$(RM) -r $(TEMPDIR)/CompileFontConfig

ifeq ($(PLATFORM), windows)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv WINDOWS
OTHER_CFLAGS += $(GX_OPTION) -DUNICODE -D_UNICODE

ifeq ($(MILESTONE), internal)
  CPPFLAGS += -DINTERNAL_BUILD
endif

#
# Useful variables that name parts of the src tree.
#
WINAWT_native        = $(PLATFORM_SRC)/native/sun/windows
WINAWT_COMMON_native = $(PLATFORM_SRC)/native/sun/awt_common

# FIXME: awt shouldn't have to see the VM's private headers.
# FIXME: if you re-order this list, the build breaks, possibly because 
#        of filename collision. Such a collision, if present, is pure
#        evil. Need to investigate this.

OTHER_INCLUDES += -I$(CLASSHDRDIR)/../../java/jvm \
	-I$(OBJDIR) \
	-I$(SHARE_SRC)/native/common \
	-I$(WINAWT_native) \
	-I$(DXSDK_INCLUDE_PATH) \
	-I$(SHARE_SRC)/native/sun/awt/image/cvutils \
	-I$(SHARE_SRC)/native/sun/awt/image \
	-I$(SHARE_SRC)/native/sun/java2d/loops \
	-I$(SHARE_SRC)/native/sun/java2d \
	-I$(PLATFORM_SRC)/native/sun/java2d \
	-I$(SHARE_SRC)/native/sun/java2d/opengl \
	-I$(PLATFORM_SRC)/native/sun/java2d/opengl \
	-I$(PLATFORM_SRC)/native/sun/java2d/windows \
	-I$(PLATFORM_SRC)/native/sun/java2d/d3d \
	-I$(SHARE_SRC)/native/sun/font \
	-I$(SHARE_SRC)/native/sun/java2d/pipe \
	-I$(SHARE_SRC)/native/sun/dc/path \
	-I$(SHARE_SRC)/native/sun/dc/doe \
	-I$(SHARE_SRC)/native/sun/awt/debug \
	-I$(PLATFORM_SRC)/native/sun/awt \
#	-I$(WINAWT_COMMON_native)

# this is only required for compiling )/native/sun/awt/medialib/*.c files
#
OTHER_INCLUDES += -I$(SHARE_SRC)/native/sun/awt/medialib

#
# .res file construction.
#

VERSIONINFO_RESOURCE = $(WINAWT_native)/awt.rc

# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ WINDOWS
endif # PLATFORM

ifneq ($(PLATFORM), windows)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv SOLARIS/LINUX

#
# Other extra flags needed for compiling.
#

ifeq ($(PLATFORM), linux)
CPPFLAGS += -I$(MOTIF_DIR)/include \
            -I$(OPENWIN_HOME)/include \
            -I$(OPENWIN_HOME)/include/X11/extensions \
            -I$(PLATFORM_SRC)/native/$(PKGDIR)/font 
endif
CPPFLAGS += -I$(SHARE_SRC)/native/$(PKGDIR)/debug \
            -I$(SHARE_SRC)/native/$(PKGDIR)/../font \
            -I$(PLATFORM_SRC)/native/$(PKGDIR)/../font \
            -I$(SHARE_SRC)/native/$(PKGDIR)/image \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/image/cvutils \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/shell \
            -I$(SHARE_SRC)/native/$(PKGDIR)/medialib \
            -I$(PLATFORM_SRC)/native/$(PKGDIR)/medialib \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/../java2d \
	    -I$(PLATFORM_SRC)/native/$(PKGDIR)/../java2d \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/../java2d/loops \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/../java2d/pipe \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/../java2d/opengl \
	    -I$(PLATFORM_SRC)/native/$(PKGDIR)/../java2d/opengl \
	    -I$(PLATFORM_SRC)/native/$(PKGDIR)/../java2d/x11 \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/../dc/doe \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/../dc/path \
            -I$(PLATFORM_SRC)/native/$(PKGDIR)/../jdga \
            -I$(PLATFORM_SRC)/native/$(PKGDIR) \
	    $(EVENT_MODEL)

ifeq ($(PLATFORM), linux)
LDFLAGS += -L$(MOTIF_LIB) -L$(OPENWIN_LIB)
endif

LDFLAGS += -L$(LIBDIR)/$(LIBARCH)/$(TSOBJDIR) \
	   $(AWT_RUNPATH)

# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ SOLARIS/LINUX
endif # PLATFORM

CLASSES.export += java.io.InputStream \
		  java.lang.Integer \
		  java.lang.ThreadGroup

#
# Has been converted to the JNI: generate JNI-style header files
#
JAVAHFLAGS += -jni

#
# Remove some files that are created in the main classes directory
# during compilation for awt character converters, these class files
# should live in charsets.jar only.
#
ifeq ($(PLATFORM), windows)
UNDESIRED_FILES = \
	$(CLASSDESTDIR)/sun/nio/cs/ext/HKSCS.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/HKSCS\$$Encoder.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/HKSCS\$$Decoder.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/DoubleByteEncoder.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/DoubleByteDecoder.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/ExtendedCharsets.class
else
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv SOLARIS/LINUX
UNDESIRED_FILES = \
	$(CLASSDESTDIR)/sun/nio/cs/ext/JIS_X_0208_Encoder.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/JIS_X_0208_Decoder.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/JIS_X_0212_Encoder.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/JIS_X_0212_Decoder.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/JIS_X_0201.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/JIS_X_0201\$$Encoder.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/JIS_X_0201\$$Decoder.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/EUC_KR.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/EUC_KR\$$Encoder.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/EUC_KR\$$Decoder.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/EUC_CN.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/EUC_CN\$$Encoder.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/EUC_CN\$$Decoder.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/EUC_TW.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/EUC_TW\$$Encoder.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/EUC_TW\$$Decoder.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/GBK.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/GBK\$$Encoder.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/GBK\$$Decoder.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/HKSCS.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/HKSCS\$$Encoder.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/HKSCS\$$Decoder.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/DoubleByteEncoder.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/DoubleByteDecoder.class \
	$(CLASSDESTDIR)/sun/nio/cs/ext/ExtendedCharsets.class
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ SOLARIS/LINUX
endif # PLATFORM

build: undesiredfiles

undesiredfiles:
	$(RM) $(UNDESIRED_FILES)


.PHONY: robot_child debughelper debughelper.clean \
	java2d.clean awt.clean motif motif.clean \
	dgalibs dgalib.clean fontconfigs fontconfigs.clean undesiredfiles


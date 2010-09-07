#
# Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.58 10/03/23
#

BUILDDIR = ../..
PACKAGE = sun.awt.X11
LIBRARY = mawt
LIBRARY_OUTPUT = xawt
LIB_LOCATION = $(LIBDIR)/$(LIBARCH)/xawt
PRODUCT = sun

include $(BUILDDIR)/common/Defs.gmk

GEN_DIR=$(GENSRCDIR)/sun/awt/X11

CLASSES_INIT += $(TEMPDIR)/.gen.wrappers $(TEMPDIR)/.gen_icons touch.wrappers

.PHONY: generated.clean

#
# Files
#
include FILES_c_unix.gmk
include FILES_export_unix.gmk
AUTO_FILES_JAVA_DIRS = sun/awt/X11
AUTO_JAVA_PRUNE = WrapperGenerator.java

LDFLAGS += -L$(OPENWIN_LIB)

ifeq ($(PLATFORM), linux)
LDFLAGS += -lpthread
dummy := $(shell $(MKDIR) -p $(LIB_LOCATION))
endif

# Since this library will be living in a subdirectory below the other libraries
#   we need to add an extra runpath so that libraries in the upper directory
#   are found at runtime.
LD_RUNPATH_EXTRAS = ..

#
# Rules.
#
include $(BUILDDIR)/common/Mapfile-vers.gmk
include $(BUILDDIR)/common/Library.gmk

#
# Add to the ambient vpath to pick up files in subdirectories
#
vpath %.c   $(SHARE_SRC)/native/sunawt/alphacomposite
vpath %.c   $(PLATFORM_SRC)/native/sun/xawt
vpath %.c   $(PLATFORM_SRC)/native/sun/awt
vpath %.c   $(SHARE_SRC)/native/sun/awt/debug
vpath %.c   $(SHARE_SRC)/native/sun/awt/image
vpath %.c   $(SHARE_SRC)/native/sun/awt/image/gif
vpath %.c   $(SHARE_SRC)/native/sun/awt/image/cvutils
vpath %.c   $(SHARE_SRC)/native/sun/awt/shell
vpath %.c   $(SHARE_SRC)/native/sun/java2d
vpath %.c   $(SHARE_SRC)/native/sun/java2d/loops
vpath %.c   $(SHARE_SRC)/native/sun/java2d/pipe
vpath %.c   $(SHARE_SRC)/native/sun/awt/medialib
vpath %.c   $(SHARE_SRC)/native/sun/awt/utility
vpath %.cpp $(SHARE_SRC)/native/sun/image
vpath %.c   $(SHARE_SRC)/native/sun/font
vpath %.c   $(PLATFORM_SRC)/native/sun/awt/robot_child
vpath %.c   $(SHARE_SRC)/native/sun/java2d/opengl
vpath %.c   $(PLATFORM_SRC)/native/sun/java2d/opengl
vpath %.c   $(PLATFORM_SRC)/native/sun/java2d/x11

OTHER_LDLIBS = $(LIBM) -lawt -lXext -lX11 -ldl \
                   $(LDFLAGS_COMMON) $(AWT_RUNPATH) $(OTHER_LDFLAGS) -lXtst -lXi

ifeq  ($(PLATFORM), solaris)
CPPFLAGS += -DFUNCPROTO=15
dummy := $(shell $(MKDIR) -p $(LIB_LOCATION))
endif

CPPFLAGS += -I$(CUPS_HEADERS_PATH)

CPPFLAGS += -DXAWT -DXAWT_HACK \
        -I$(TEMPDIR)/../../sun.awt/awt/CClassHeaders \
        -I$(PLATFORM_SRC)/native/sun/awt \
	-I$(PLATFORM_SRC)/native/sun/xawt \
        -I$(PLATFORM_SRC)/native/sun/jdga \
        -I$(SHARE_SRC)/native/sun/awt/debug \
        -I$(SHARE_SRC)/native/sun/awt/image/cvutils \
        -I$(SHARE_SRC)/native/sun/java2d \
	-I$(SHARE_SRC)/native/sun/java2d/loops \
	-I$(SHARE_SRC)/native/sun/awt/image/cvutils \
	-I$(SHARE_SRC)/native/sun/awt/image \
	-I$(SHARE_SRC)/native/sun/font \
	-I$(PLATFORM_SRC)/native/sun/java2d \
	-I$(SHARE_SRC)/native/sun/java2d/pipe \
	-I$(SHARE_SRC)/native/sun/java2d/opengl \
	-I$(PLATFORM_SRC)/native/sun/java2d/opengl \
	-I$(PLATFORM_SRC)/native/sun/java2d/x11 \
	-I$(SHARE_SRC)/native/sun/dc/path \
	-I$(SHARE_SRC)/native/sun/dc/doe \
	-I$(SHARE_SRC)/native/sun/awt/alphacomposite \
        -I$(SHARE_SRC)/native/sun/awt/medialib \
	-I$(PLATFORM_SRC)/native/sun/awt/medialib \
        -I$(PLATFORM_SRC)/native/sun/font \
        -I$(SHARE_SRC)/native/sun/awt \
        -I$(PLATFORM_SRC)/native/sun/awt 

# Allows for builds on Debian GNU Linux, X11 is in a different place 
CPPFLAGS += -I$(firstword $(wildcard $(OPENWIN_HOME)/include/X11/extensions) \
                          $(wildcard /usr/include/X11/extensions))

CPPFLAGS += -I$(OPENWIN_HOME)/include \
            -I$(MOTIF_DIR)/include 

ifeq ($(MILESTONE), internal)
  CPPFLAGS += -DINTERNAL_BUILD
endif

#
# Has been converted to the JNI: generate JNI-style header files
#
JAVAHFLAGS += -jni

#LDFLAGS_OPT += -L../../../build/solaris-sparc/lib/sparc/motif21 -lmawt_g -lXext -lXm -lXt -ldl

#
# Generation of wrappers
#
SIZER_DIR=$(GEN_DIR)/generator
SIZER = $(SIZER_DIR)/sizer
SIZER_32_C = $(SIZER).32.c
SIZER_64_C = $(SIZER).64.c
CFLAGS_32=$(CFLAGS)
CFLAGS_64=$(CFLAGS)

ifeq ($(PLATFORM), solaris)

isalist:=$(shell $(ISAINFO))

ifneq (,$(findstring sparcv9, $(isalist)))
# On sparcv9 we generate both 32 and 64-bit sizers in spite of ARCH_DATA_MODEL.
# On sparcv9 CFLAGS already contain $(XARCH_OPTION/64), so to generate 32-bit sizer we need to change this option.
CFLAGS_32=$(subst $(XARCH_OPTION/64),$(XARCH_OPTION/32),$(CFLAGS))
SIZERS = $(SIZER).32 $(SIZER).64
SIZERS_C = $(SIZER_32_C) $(SIZER_64_C)
SIZES = $(WRAPPER_GENERATOR_DIR)/sizes.32 $(WRAPPER_GENERATOR_DIR)/sizes.64
CFLAGS_64=$(CFLAGS) $(XARCH_OPTION/64)
else
ifneq (,$(findstring amd64, $(isalist)))
# On amd64 we generate both 32 and 64-bit sizers in spite of ARCH_DATA_MODEL.
# On amd64 CFLAGS already contain $(XARCH_OPTION/64), so to generate 32-bit sizer we need to change this option.
CFLAGS_32=$(subst $(XARCH_OPTION/64),$(XARCH_OPTION/32),$(CFLAGS))
SIZERS = $(SIZER).32 $(SIZER).64
SIZERS_C = $(SIZER_32_C) $(SIZER_64_C)
SIZES = $(WRAPPER_GENERATOR_DIR)/sizes.32 $(WRAPPER_GENERATOR_DIR)/sizes.64
CFLAGS_64=$(CFLAGS) $(XARCH_OPTION/64)
else # !sparcv9 : includes (32-bit) sparc, i586
SIZERS = $(SIZER).32
SIZERS_C = $(SIZER_32_C)
SIZES = $(WRAPPER_GENERATOR_DIR)/sizes.32
endif # amd64
endif # sparcv9

else # !solaris

ifeq ($(ARCH_DATA_MODEL), 32)
SIZERS = $(SIZER).32
SIZERS_C = $(SIZER_32_C)
SIZES = $(WRAPPER_GENERATOR_DIR)/sizes.32
else # !32
SIZERS = $(SIZER).64
SIZERS_C = $(SIZER_64_C)
SIZES = $(WRAPPER_GENERATOR_DIR)/sizes.64
endif # 32

endif # solaris

# XXX Hack for 6185483 - use hard-coded sizes.
# Add the 64-bit platforms that need to be included into 32-bit build
# and have sizes.64-$(PLATFORM)-$(LIBARCH) hardcoded in the workspace
ifeq ($(PLATFORM)-$(LIBARCH), solaris-i386)
# If you define DOHACK=true for some combination of $(PLATFORM)-$(LIBARCH),
# make sure you have sizes.64-$(PLATFORM)-$(LIBARCH) pre-generated in
# $(PLATFORM_SRC)/classes/sun/awt/X11/generator/
DOHACK=true
endif # Hack

ifeq ($(DOHACK), true)
PREDEFINED_SIZES_TMPL=$(PLATFORM_SRC)/classes/sun/awt/X11/generator/sizes.64-$(PLATFORM)-$(LIBARCH)
PREDEFINED_SIZES=$(WRAPPER_GENERATOR_DIR)/sizes.64
SIZES += $(WRAPPER_GENERATOR_DIR)/sizes.64
endif

WRAPPER_GENERATOR_JAVA=$(PLATFORM_SRC)/classes/sun/awt/X11/generator/WrapperGenerator.java
WRAPPER_GENERATOR_DIR=$(GENSRCDIR)/sun/awt/X11/generator
WRAPPER_GENERATOR_CLASS=$(WRAPPER_GENERATOR_DIR)/WrapperGenerator.class 
XLIBTYPES=$(PLATFORM_SRC)/classes/sun/awt/X11/generator/xlibtypes.txt

$(SIZERS): $(SIZERS_C) 
	$(prep-target)
	$(CC) $(CFLAGS_$(subst .,,$(suffix $@))) $(CPPFLAGS) -o $@ $(SIZER)$(suffix $@).c

$(WRAPPER_GENERATOR_CLASS): $(WRAPPER_GENERATOR_JAVA)
	$(MKDIR) -p $(WRAPPER_GENERATOR_DIR)
	$(JAVAC_CMD) -d $(WRAPPER_GENERATOR_DIR) $(WRAPPER_GENERATOR_JAVA)

$(SIZERS_C): $(WRAPPER_GENERATOR_CLASS) $(XLIBTYPES)
	$(prep-target)
	$(JAVA) -classpath $(WRAPPER_GENERATOR_DIR) WrapperGenerator $(SIZER_DIR) $(XLIBTYPES) "sizer" $(subst .,,$(suffix $(basename $@)))

$(SIZES): $(SIZERS)
	@if [ "$(DOHACK)$@" = "true$(PREDEFINED_SIZES)" ]; then \
		$(ECHO) COPYING $(PREDEFINED_SIZES_TMPL) into $@; $(CP) $(PREDEFINED_SIZES_TMPL) $@; $(CHMOD) +w $@;\
	else	$(ECHO) GENERATING $@; $(WRAPPER_GENERATOR_DIR)/sizer$(suffix $@) > $@; fi

$(TEMPDIR)/.gen.wrappers: $(SIZES) $(WRAPPER_GENERATOR_CLASS) $(XLIBTYPES)
	$(JAVA) -classpath $(WRAPPER_GENERATOR_DIR) WrapperGenerator $(GEN_DIR) $(XLIBTYPES) "gen" $(WRAPPER_GENERATOR_DIR)/sizes

touch.wrappers: $(TEMPDIR)/.gen.wrappers
	$(MKDIR) -p $(TEMPDIR)
	$(TOUCH) $(TEMPDIR)/.gen.wrappers

generated.clean:	
	$(RM) -r $(WRAPPER_GENERATOR_DIR)
	$(RM) -r $(GEN_DIR)/*.java

ICONPATH=$(PLATFORM_SRC)/classes/sun/awt/X11

ICONS=$(ICONPATH)/java-icon16.png $(ICONPATH)/java-icon24.png $(ICONPATH)/java-icon32.png $(ICONPATH)/java-icon48.png

ICONS += $(ICONPATH)/security-icon-bw16.png $(ICONPATH)/security-icon-interim16.png $(ICONPATH)/security-icon-yellow16.png $(ICONPATH)/security-icon-bw24.png $(ICONPATH)/security-icon-interim24.png $(ICONPATH)/security-icon-yellow24.png $(ICONPATH)/security-icon-bw32.png $(ICONPATH)/security-icon-interim32.png $(ICONPATH)/security-icon-yellow32.png $(ICONPATH)/security-icon-bw48.png $(ICONPATH)/security-icon-interim48.png $(ICONPATH)/security-icon-yellow48.png

$(TARGDIR)sun/awt/X11/ToBin.class: ToBin.java
	$(JAVAC_BOOT_CMD) $?

$(TEMPDIR)/.gen_icons: $(TARGDIR)sun/awt/X11/ToBin.class $(ICONS)
	$(prep-target)
	for i in $(ICONS); do \
            filename=`basename $$i`; \
	    name=`$(ECHO) $$filename | $(TR) '\-.' '__'`;  \
            classname=$(GEN_DIR)/XAWTIcon32_$$name.java; \
	    $(RM) $$classname; \
	    $(ECHO) "package sun.awt.X11; public class XAWTIcon32_$$name {" >> $$classname; \
	    $(ECHO) "public static int[] $$name = { " >> $$classname;  \
	    $(CAT) $$i | $(JAVA_BOOT) -classpath ${CLASSBINDIR}  -Dawt.toolkit=sun.awt.motif.MToolkit -Djava.awt.headless=true sun.awt.X11.ToBin >> $$classname; \
	    $(ECHO) "}; }" >> $$classname;  \
            classname=$(GEN_DIR)/XAWTIcon64_$$name.java; \
	    $(RM) $$classname; \
	    $(ECHO) "package sun.awt.X11; public class XAWTIcon64_$$name {" >> $$classname; \
	    $(ECHO) "public static long[] $$name = { " >> $$classname;  \
	    $(CAT) $$i | $(JAVA_BOOT) -classpath ${CLASSBINDIR}  -Dawt.toolkit=sun.awt.motif.MToolkit -Djava.awt.headless=true sun.awt.X11.ToBin >> $$classname; \
	    $(ECHO) "}; }" >> $$classname;  \
	done
	$(TOUCH) $@

clean clobber:: generated.clean

.PHONY: generated.clean robot_child


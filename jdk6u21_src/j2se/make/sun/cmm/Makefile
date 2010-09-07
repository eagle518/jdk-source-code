#
# @(#)Makefile	1.42 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
PACKAGE = sun.awt
LIBRARY = cmm
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
ifeq ($(PLATFORM), windows)
include FILES_c_windows.gmk
else # PLATFORM
include FILES_c_unix.gmk
endif # PLATFORM

FILES_java = \
	sun/awt/color/CMM.java

FILES_export = \
	sun/awt/color/CMM.java

ifeq ($(PLATFORM), windows)
  # Override the default version info with our own resource file (see 5043594)
  VERSIONINFO_RESOURCE = $(SHARE_SRC)/native/sun/awt/cmm/cmm.rc
  # Files built here do not compile with warning level 3 if warnings are fatal
  COMPILER_WARNINGS_FATAL=false
endif

#
# Rules
#
include $(BUILDDIR)/common/Mapfile-vers.gmk
include $(BUILDDIR)/common/Library.gmk

build: iccprofiles

#
# Extra rules
#
ifeq  ($(PLATFORM), linux)
LDLIBS += -lpthread
endif

clean clobber:: iccprofiles.clean

#
# Add to ambient vpath to get files in a subdirectory
#
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/cmm

ifeq ($(PLATFORM), windows)

OTHER_LDLIBS = user32.lib version.lib

clean:: res.clean

res.clean:
	$(RM) $(OBJDIR)/cmm.rc
	$(RM) $(OBJDIR)/cmm.res
	$(RM) $(OBJDIR)/cmm.h

else # PLATFORM
OTHER_LDLIBS = $(LIBM)

# Use pic instead of PIC on this library
GLOBAL_KPIC = $(PIC_CODE_SMALL)

CPPFLAGS += -I$(CLASSHDRDIR) \

endif # PLATFORM

#CFLAGS += -DJAVACMM -DFUT_CALC_EX -DNO_FUT_GCONST
CFLAGS += -DFUT_CALC_EX -DNO_FUT_GCONST

#
# Has been converted to the JNI: generate JNI-style header files
#
JAVAHFLAGS += -jni

#
# ICC Profiles used by the CMM.
#
ICCPROFILE_SRC_DIR  = $(SHARE_SRC)/lib/cmm
ICCPROFILE_DEST_DIR = $(LIBDIR)/cmm

iccprofiles: $(ICCPROFILE_DEST_DIR)/sRGB.pf $(ICCPROFILE_DEST_DIR)/GRAY.pf \
	     $(ICCPROFILE_DEST_DIR)/CIEXYZ.pf $(ICCPROFILE_DEST_DIR)/PYCC.pf \
	     $(ICCPROFILE_DEST_DIR)/LINEAR_RGB.pf

$(ICCPROFILE_DEST_DIR)/sRGB.pf: $(ICCPROFILE_SRC_DIR)/sRGB.pf
	$(RM) $(ICCPROFILE_DEST_DIR)/sRGB.pf
	-$(MKDIR) -p $(ICCPROFILE_DEST_DIR)
	$(CP) $(ICCPROFILE_SRC_DIR)/sRGB.pf $(ICCPROFILE_DEST_DIR)
	$(CHMOD) 444 $(ICCPROFILE_DEST_DIR)/sRGB.pf

$(ICCPROFILE_DEST_DIR)/GRAY.pf: $(ICCPROFILE_SRC_DIR)/GRAY.pf
	$(RM) $(ICCPROFILE_DEST_DIR)/GRAY.pf
	-$(MKDIR) -p $(ICCPROFILE_DEST_DIR)
	$(CP) $(ICCPROFILE_SRC_DIR)/GRAY.pf $(ICCPROFILE_DEST_DIR)
	$(CHMOD) 444 $(ICCPROFILE_DEST_DIR)/GRAY.pf

$(ICCPROFILE_DEST_DIR)/CIEXYZ.pf: $(ICCPROFILE_SRC_DIR)/CIEXYZ.pf
	$(RM) $(ICCPROFILE_DEST_DIR)/CIEXYZ.pf
	-$(MKDIR) -p $(ICCPROFILE_DEST_DIR)
	$(CP) $(ICCPROFILE_SRC_DIR)/CIEXYZ.pf $(ICCPROFILE_DEST_DIR)
	$(CHMOD) 444 $(ICCPROFILE_DEST_DIR)/CIEXYZ.pf

$(ICCPROFILE_DEST_DIR)/PYCC.pf: $(ICCPROFILE_SRC_DIR)/PYCC.pf
	$(RM) $(ICCPROFILE_DEST_DIR)/PYCC.pf
	-$(MKDIR) -p $(ICCPROFILE_DEST_DIR)
	$(CP) $(ICCPROFILE_SRC_DIR)/PYCC.pf $(ICCPROFILE_DEST_DIR)
	$(CHMOD) 444 $(ICCPROFILE_DEST_DIR)/PYCC.pf

$(ICCPROFILE_DEST_DIR)/LINEAR_RGB.pf: $(ICCPROFILE_SRC_DIR)/LINEAR_RGB.pf
	$(RM) $(ICCPROFILE_DEST_DIR)/LINEAR_RGB.pf
	-$(MKDIR) -p $(ICCPROFILE_DEST_DIR)
	$(CP) $(ICCPROFILE_SRC_DIR)/LINEAR_RGB.pf $(ICCPROFILE_DEST_DIR)
	$(CHMOD) 444 $(ICCPROFILE_DEST_DIR)/LINEAR_RGB.pf

iccprofiles.clean:
	$(RM) -r $(ICCPROFILE_DEST_DIR)

.PHONY: res.clean iccprofiles iccprofiles.clean


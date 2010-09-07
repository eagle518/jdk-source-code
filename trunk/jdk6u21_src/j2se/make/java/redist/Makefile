#
# @(#)Makefile	1.86 03/11/19
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Imports files exported by a hotspot build or provided from an external
#   location into the OUTPUTDIR, and also primes the OUTPUTDIR with files
#   that are provided inside this workspace.
#
# IMPORT_LIST contains the list of destination files that are copied 
#     from external places (outside this workspace).
#
# INTERNAL_IMPORT_LIST is the list of destination files from BUILDDIR.
# 
# JARS_TO_EXPLODE is the name of jar files relative to BUILDDIR that we
#     need to extract classes from and place in the outputdir classes directory.
#
# JAR_PACKAGES_TO_COPY are the package names from the above jar files to copy.
#

BUILDDIR = ../..
include $(BUILDDIR)/common/Defs.gmk
PRODUCT  = java

SERVER_LOCATION = server
CLIENT_LOCATION = client

DB_SUFFIX = _db

ifeq ($(PLATFORM), windows)
  LIB_LOCATION = $(BINDIR)
else
  LIB_LOCATION = $(LIBDIR)/$(LIBARCH)
endif

JVM_NAME               = $(LIB_PREFIX)jvm.$(LIBRARY_SUFFIX)
JVMLIB_NAME            = $(LIB_PREFIX)jvm.$(LIB_SUFFIX)
JVMMAP_NAME            = $(LIB_PREFIX)jvm.map
JVMPDB_NAME            = $(LIB_PREFIX)jvm.pdb
LIBJSIG_NAME           = $(LIB_PREFIX)jsig.$(LIBRARY_SUFFIX)
JVMDB_NAME             = $(LIB_PREFIX)jvm$(DB_SUFFIX).$(LIBRARY_SUFFIX)

SHARINGDIR             = $(BUILDDIR)/tools/sharing

include $(BUILDDIR)/common/Defs.gmk

default_rule: all

# Needed to do file copy
ABS_BUILDDIR :=$(call FullPath,$(BUILDDIR))

# Temporary directory for AddJsum classes.
JSUM_CLASSDIR = $(TEMPDIR)/jsum

# Jar files to explode (relative paths)
JARS_TO_EXPLODE = tools/crypto/jgss/jgss_kerberos.jar

# Package names to copy
JAR_PACKAGES_TO_COPY = sun/security/krb5 sun/security/jgss/spi

# List of files created here or coming from BUILDDIR area (this workspace)
INTERNAL_IMPORT_LIST = \
            $(LIBDIR)/security/US_export_policy.jar \
            $(LIBDIR)/security/local_policy.jar \
            $(LIBDIR)/jce.jar \
            $(EXTDIR)/sunjce_provider.jar \
            $(LIBDIR)/jsse.jar \
	    $(LIBDIR)/classlist

# List of files coming from outside this workspace
IMPORT_LIST = $(LIB_LOCATION)/$(SERVER_LOCATION)/$(JVM_NAME) \
              $(LIB_LOCATION)/$(SERVER_LOCATION)/Xusage.txt

# INCLUDE_SA is false on platforms where SA is not supported.
# On platforms where it is supported, we want to allow it to
# not be present, at least temporarily.  So,
# if the SA files (well, just sa-jdi.jar) do not exist 
# in the HOTSPOT_IMPORT_PATH, then we won't build SA.
SA_EXISTS := $(shell if [ -r $(HOTSPOT_IMPORT_PATH)/lib/sa-jdi.jar ]; then \
                          $(ECHO) true; \
                        else \
                          $(ECHO) false; \
                        fi)

ifeq ($(SA_EXISTS), false)
  INCLUDE_SA := false
endif

ifeq ($(INCLUDE_SA), true)
  IMPORT_LIST += $(LIBDIR)/sa-jdi.jar \
                 $(LIB_LOCATION)/$(SALIB_NAME)
  ifeq ($(PLATFORM), windows)
    IMPORT_LIST += $(LIB_LOCATION)/$(SAMAP_NAME) \
                   $(LIB_LOCATION)/$(SAPDB_NAME)
  endif
endif # INCLUDE_SA

# Hotspot client is only available on 32-bit builds
ifeq ($(ARCH_DATA_MODEL), 32)
  IMPORT_LIST += $(LIB_LOCATION)/$(CLIENT_LOCATION)/$(JVM_NAME) \
                 $(LIB_LOCATION)/$(CLIENT_LOCATION)/Xusage.txt
endif

ifeq ($(PLATFORM), linux)

LIBCHMOD_NAME  = $(LIB_PREFIX)native_chmod.$(LIBRARY_SUFFIX)

INTERNAL_IMPORT_LIST += $(LIBDIR)/$(LIBARCH)/$(LIBCHMOD_NAME)

NATIVE_CHMOD_ZIP = $(BUILDDIR)/tools/crypto/jgss/$(ARCH)/native_chmod.tar.gz

$(LIBDIR)/$(LIBARCH)/$(LIBCHMOD_NAME): $(NATIVE_CHMOD_ZIP)
	@$(prep-target)
	($(GUNZIP) < $<) | ($(CD) $(@D); $(TAR) xf -)

endif # linux

ifeq ($(PLATFORM), windows)
#  Windows     vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv  Windows

INTERNAL_IMPORT_LIST += $(BINDIR)/w2k_lsa_auth.dll

W2K_LSA_AUTH_ZIP = $(BUILDDIR)/tools/crypto/jgss/$(ARCH)/w2k_lsa_auth.zip

$(BINDIR)/w2k_lsa_auth.dll: $(W2K_LSA_AUTH_ZIP)
	@$(prep-target)
	$(UNZIP) $< $(@F) -d $(BINDIR)
	$(CHMOD) a+x $@

IMPORT_LIST += $(MS_RUNTIME_LIBRARIES:%=$(BINDIR)/%)

# NOTE: These might actually come from BUILDDIR, depends on the settings.
$(BINDIR)/msvcrt.dll: $(MSVCRT_DLL_PATH)/msvcrt.dll
	$(install-file)
	$(CHMOD) a+x $@
$(BINDIR)/unicows.dll: $(UNICOWS_DLL_PATH)/unicows.dll
	$(install-file)
	$(CHMOD) a+x $@
$(BINDIR)/msvcr71.dll: $(MSVCR71_DLL_PATH)/msvcr71.dll
	$(install-file)
	$(CHMOD) a+x $@

# Get the hotspot .map and .pdb files for client and server
IMPORT_LIST += \
	$(LIBDIR)/$(JVMLIB_NAME) \
	$(LIB_LOCATION)/$(SERVER_LOCATION)/$(JVMMAP_NAME) \
	$(LIB_LOCATION)/$(SERVER_LOCATION)/$(JVMPDB_NAME)

# Hotspot client is only available on 32-bit builds
ifeq ($(ARCH_DATA_MODEL), 32)
  IMPORT_LIST += $(LIB_LOCATION)/$(CLIENT_LOCATION)/$(JVMMAP_NAME) \
                 $(LIB_LOCATION)/$(CLIENT_LOCATION)/$(JVMPDB_NAME)
endif

$(LIBDIR)/$(JVMLIB_NAME): $(HOTSPOT_LIB_PATH)/$(JVMLIB_NAME)
	$(install-file)

# it is OK for the .map and .pdb files to not exist, so do not force a 
# dependency on them from the bootstrap location, and allow the copy to fail.
$(LIB_LOCATION)/$(CLIENT_LOCATION)/$(JVMMAP_NAME):
	@$(prep-target)
	-$(CP) $(HOTSPOT_CLIENT_PATH)/$(JVMMAP_NAME)  $@

$(LIB_LOCATION)/$(SERVER_LOCATION)/$(JVMMAP_NAME):
	@$(prep-target)
	-$(CP) $(HOTSPOT_SERVER_PATH)/$(JVMMAP_NAME) $@

$(LIB_LOCATION)/$(CLIENT_LOCATION)/$(JVMPDB_NAME):
	@$(prep-target)
	-$(CP) $(HOTSPOT_CLIENT_PATH)/$(JVMPDB_NAME)  $@

$(LIB_LOCATION)/$(SERVER_LOCATION)/$(JVMPDB_NAME): 
	@$(prep-target)
	-$(CP) $(HOTSPOT_SERVER_PATH)/$(JVMPDB_NAME) $@

# enable kerberos tools for windows
JARS_TO_EXPLODE += tools/crypto/jgss/ktools.jar
JAR_PACKAGES_TO_COPY += sun/security/krb5/internal/tools

#  Windows     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^  Windows
else # PLATFORM
#  NOT Windows vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv  NOT Windows

IMPORT_LIST += \
        $(LIB_LOCATION)/$(LIBJSIG_NAME) \
	$(LIB_LOCATION)/$(SERVER_LOCATION)/$(LIBJSIG_NAME)

ifeq ($(PLATFORM), solaris)
  IMPORT_LIST += $(LIB_LOCATION)/$(SERVER_LOCATION)/$(JVMDB_NAME)
endif 

ifeq ($(ARCH_DATA_MODEL), 32)

IMPORT_LIST += $(LIB_LOCATION)/$(CLIENT_LOCATION)/$(LIBJSIG_NAME)

ifeq ($(PLATFORM), solaris)
#  solaris   vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv  solaris

ifeq ($(ARCH), sparc)
  IMPORT_LIST += $(LIB_LOCATION)/$(SERVER_LOCATION)/64/$(JVMDB_NAME)
endif

IMPORT_LIST += $(LIB_LOCATION)/$(CLIENT_LOCATION)/$(JVMDB_NAME)

ifeq ($(ARCH), sparc)
  IMPORT_LIST += $(LIB_LOCATION)/$(CLIENT_LOCATION)/64/$(JVMDB_NAME)
endif

# For backwards compatability, make a link of the 32-bit client JVM to $(LIBDIR)
IMPORT_LIST += $(LIB_LOCATION)/$(JVM_NAME)

# create a link from lib/libjvm.so to client/libjvm.so
$(LIB_LOCATION)/$(JVM_NAME): $(LIB_LOCATION)/$(CLIENT_LOCATION)/$(JVM_NAME)
	@$(prep-target)
	$(LN) -s $(CLIENT_LOCATION)/$(JVM_NAME) $@

#  solaris   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ solaris
endif # 32bit solaris

endif # 32bit

#  NOT Windows ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ NOT Windows

endif # PLATFORM

$(LIB_LOCATION)/$(CLIENT_LOCATION)/$(JVM_NAME): $(HOTSPOT_CLIENT_PATH)/$(JVM_NAME)
	$(install-file)

$(LIB_LOCATION)/$(LIBJSIG_NAME): $(HOTSPOT_IMPORT_PATH)/$(ARCH_VM_SUBDIR)/$(LIBJSIG_NAME)
	$(install-file)

$(LIB_LOCATION)/$(CLIENT_LOCATION)/$(LIBJSIG_NAME) \
$(LIB_LOCATION)/$(SERVER_LOCATION)/$(LIBJSIG_NAME):
	@$(prep-target)
	$(LN) -s ../$(LIBJSIG_NAME) $@


# libjvm_db.so is only on Solaris

ifeq ($(PLATFORM), solaris)
$(LIB_LOCATION)/$(CLIENT_LOCATION)/$(JVMDB_NAME): $(HOTSPOT_CLIENT_PATH)/$(JVMDB_NAME)
	$(install-file)

$(LIB_LOCATION)/$(CLIENT_LOCATION)/64/$(JVMDB_NAME): $(HOTSPOT_CLIENT_PATH)/64/$(JVMDB_NAME)
	$(install-file)

$(LIB_LOCATION)/$(SERVER_LOCATION)/$(JVMDB_NAME): $(HOTSPOT_SERVER_PATH)/$(JVMDB_NAME)
	$(install-file)

$(LIB_LOCATION)/$(SERVER_LOCATION)/64/$(JVMDB_NAME): $(HOTSPOT_SERVER_PATH)/64/$(JVMDB_NAME)
	$(install-file)
endif # platform solaris


$(LIB_LOCATION)/$(SERVER_LOCATION)/$(JVM_NAME): $(HOTSPOT_SERVER_PATH)/$(JVM_NAME)
	$(install-file)

$(LIB_LOCATION)/$(SERVER_LOCATION)/Xusage.txt : $(HOTSPOT_SERVER_PATH)/Xusage.txt
	$(install-file)

$(LIB_LOCATION)/$(CLIENT_LOCATION)/Xusage.txt : $(HOTSPOT_CLIENT_PATH)/Xusage.txt
	$(install-file)

ifeq ($(INCLUDE_SA), true)
# The Serviceability Agent is built in the Hotspot workspace.
# It contains two files:
#  - sa-jdi.jar:  This goes into the same dir as tools.jar.
#  - a shared library:  sawindbg.dll on windows / libproc.sa on unix
#		        This goes into the same dir as the other
#			shared libs, eg. libjdwp.so.
$(LIBDIR)/sa-jdi.jar: $(HOTSPOT_IMPORT_PATH)/lib/sa-jdi.jar
	$(install-file)

$(LIB_LOCATION)/$(SALIB_NAME): $(HOTSPOT_SALIB_PATH)/$(SALIB_NAME)
	$(install-file)

ifeq ($(PLATFORM), windows)
$(LIB_LOCATION)/$(SAPDB_NAME): $(HOTSPOT_SALIB_PATH)/$(SAPDB_NAME)
	$(install-file)

$(LIB_LOCATION)/$(SAMAP_NAME): $(HOTSPOT_SALIB_PATH)/$(SAMAP_NAME)
	$(install-file)
endif # windows
endif # INCLUDE_SA

# Internal import files
$(LIBDIR)/jce.jar: $(BUILDDIR)/tools/crypto/jce/jce.jar
	$(install-file)

$(EXTDIR)/sunjce_provider.jar: $(BUILDDIR)/tools/crypto/jce/sunjce_provider.jar
	$(install-file)

$(LIBDIR)/security/US_export_policy.jar: $(BUILDDIR)/tools/crypto/jce/US_export_policy.jar
	$(install-file)

$(LIBDIR)/security/local_policy.jar: $(BUILDDIR)/tools/crypto/jce/local_policy.jar
	$(install-file)

$(LIBDIR)/jsse.jar: $(BUILDDIR)/tools/crypto/jsse/jsse.jar
	$(install-file)

$(JSUM_CLASSDIR)/AddJsum.class:  $(SHARINGDIR)/AddJsum.java
	@$(prep-target)
	$(JAVAC_BOOT) -d $(@D) $<

# Construct classlist file
$(LIBDIR)/classlist: $(SHARINGDIR)/classlist.$(PLATFORM) $(JSUM_CLASSDIR)/AddJsum.class
	@$(prep-target)
	@$(RM) -f $@.temp
	$(JAVA_BOOT) -cp $(JSUM_CLASSDIR) AddJsum $(SHARINGDIR)/classlist.$(PLATFORM) $@.temp
	$(MV) $@.temp $@

# Get names of font files
include FILES.gmk

# Copy font files into OUTPUTDIR area

FONTFILES = $(SHARED_FONTFILES)
FONTSDIR  = $(LIBDIR)/fonts
FONTSDIRFILE  = $(FONTSDIR)/fonts.dir
INTERNAL_IMPORT_LIST += $(FONTFILES) 

ifneq ($(PLATFORM), windows)
  INTERNAL_IMPORT_LIST += $(FONTSDIRFILE)
endif

$(LIBDIR)/fonts/%.ttf: $(SHARE_SRC)/lib/fonts/%.ttf
	$(install-file)

$(FONTSDIRFILE): $(PLATFORM_SRC)/classes/sun/awt/motif/java.fonts.dir
	$(install-file)

ifeq ($(PLATFORM), linux)

# The oblique fonts are only needed/wanted on Linux.

OBLFONTSDIR  = $(LIBDIR)/oblique-fonts
OBLFONTSDIRFILE	 = $(OBLFONTSDIR)/fonts.dir
INTERNAL_IMPORT_LIST += $(OBLIQUE_FONTFILES) $(OBLFONTSDIRFILE)

$(LIBDIR)/oblique-fonts/%.ttf: $(SHARE_SRC)/lib/fonts/oblique/%.ttf
	$(install-file)

$(OBLFONTSDIRFILE): $(PLATFORM_SRC)/classes/sun/awt/motif/java.oblique-fonts.dir
	$(install-file)

endif # linux

# Absolute paths to jars we need to explode
MASTER_JARS = $(JARS_TO_EXPLODE:%.jar=$(ABS_BUILDDIR)/%.jar)

# Pick a few classes that if they disappear we need to reinstall
#   ideally all the files would be listed here, I just picked a few.
PARTIAL_CLASSFILE_LIST = \
    $(CLASSBINDIR)/sun/security/krb5/Config.class \
    $(CLASSBINDIR)/sun/security/jgss/spi/GSSContextSpi.class

# Install all classes from the jar files if any classfile in list is missing
$(PARTIAL_CLASSFILE_LIST): $(MASTER_JARS)
	@$(MKDIR) -p $(CLASSBINDIR)
	@for i in $(MASTER_JARS) ; do \
	    $(ECHO) " $(CD) $(CLASSBINDIR) && \
		$(JAR_BOOT) xvf $${i} $(JAR_PACKAGES_TO_COPY) $(JAR_JFLAGS)"; \
	    $(CD) $(CLASSBINDIR) && \
		$(JAR_BOOT) xvf $${i} $(JAR_PACKAGES_TO_COPY) $(JAR_JFLAGS); \
	done

import_classes : $(PARTIAL_CLASSFILE_LIST)

all : build

import_internal_files : $(INTERNAL_IMPORT_LIST)

import_files : $(IMPORT_LIST)

build : import_files import_internal_files import_classes

clean_classes :
	@if [ -d $(CLASSBINDIR) ]; then \
	    $(ECHO) "$(CD) $(CLASSBINDIR) && $(RM) -r $(JAR_PACKAGES_TO_COPY)";\
	    $(CD) $(CLASSBINDIR) && $(RM) -r $(JAR_PACKAGES_TO_COPY); \
	fi

clean clobber :: clean_classes
	$(RM) -r $(JSUM_CLASSDIR)
	$(RM) $(IMPORT_LIST)
	$(RM) $(INTERNAL_IMPORT_LIST)

.PHONY: import_files import_internal_files import_classes


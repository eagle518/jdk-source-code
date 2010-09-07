#
# @(#)Makefile	1.116 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for core Java libraries
# (java.lang, java.lang.ref, java.lang.reflect, java.io, java.util)
#

BUILDDIR = ../..
PACKAGE = java.lang
LIBRARY = java
PRODUCT = java
include $(BUILDDIR)/common/Defs.gmk

# windows compiler flags
ifeq ($(PLATFORM),windows)
  OTHER_CFLAGS =
  # Files built here do not compile with warning level 3 if warnings are fatal
  COMPILER_WARNINGS_FATAL=false
  # build directly into BINDIR...
  LIB_LOCATION = $(BINDIR)
  # Exported functions
  OTHER_LCF = -export:winFileHandleOpen -export:handleLseek
endif

OTHER_CFLAGS += -DJDK_MAJOR_VERSION='"$(JDK_MAJOR_VERSION)"' \
                -DJDK_MINOR_VERSION='"$(JDK_MINOR_VERSION)"' \
                -DJDK_MICRO_VERSION='"$(JDK_MICRO_VERSION)"' \
                -DJDK_BUILD_NUMBER='"$(JDK_BUILD_NUMBER)"' 

ifdef JDK_UPDATE_VERSION
OTHER_CFLAGS += -DJDK_UPDATE_VERSION='"$(JDK_UPDATE_VERSION)"'
endif


#
# Files to compile.
#
include FILES_c.gmk
include FILES_java.gmk
include Exportedfiles.gmk

ifeq ($(PLATFORM),windows)
FILES_java += 	$(TARGDIR)java/io/Win32FileSystem.java \
		$(TARGDIR)java/io/WinNTFileSystem.java \
		$(TARGDIR)java/util/prefs/WindowsPreferences.java \
                $(TARGDIR)java/util/prefs/WindowsPreferencesFactory.java

FILES_c    +=   $(TARGDIR)ProcessImpl_md.c \
		$(TARGDIR)Win32FileSystem_md.c \
		$(TARGDIR)WinNTFileSystem_md.c \
		$(TARGDIR)canonicalize_md.c \
		$(TARGDIR)dirent_md.c \
		$(TARGDIR)TimeZone.c \
		$(TARGDIR)TimeZone_md.c \
		$(TARGDIR)WindowsPreferences.c \
		$(TARGDIR)sun/security/provider/WinCAPISeedGenerator.c \
		$(TARGDIR)sun/io/Win32ErrorMode.c

else # PLATFORM
FILES_java += 	$(TARGDIR)java/lang/UNIXProcess.java \
		$(TARGDIR)java/io/UnixFileSystem.java \
		$(TARGDIR)java/util/prefs/FileSystemPreferences.java \
                $(TARGDIR)java/util/prefs/FileSystemPreferencesFactory.java \

FILES_c    +=   $(TARGDIR)UNIXProcess_md.c \
		$(TARGDIR)UnixFileSystem_md.c \
		$(TARGDIR)canonicalize_md.c \
		$(TARGDIR)TimeZone.c \
		$(TARGDIR)TimeZone_md.c \
		$(TARGDIR)FileSystemPreferences.c

INIT += $(GENSRCDIR)/java/lang/UNIXProcess.java

endif # PLATFORM

#
# Make sure first rule does 'all'
#
default_rule: all

#
# Source files to generate before we try to compile anything
#

#
# genlocales.gmk define rules to generate LocaleDataMetaInfo.java
# which contains locale string list for sun.text.resources and
# sun.util.resources.
#
include genlocales.gmk

CLASSES_INIT = gencharsrc niosrc rbcontrolsrc genlocales

niosrc: ; ($(CD) ../nio; $(MAKE) sources)

gencharsrc: $(GENSRCDIR)/java/lang/CharacterData00.java \
    $(GENSRCDIR)/java/lang/CharacterData01.java \
    $(GENSRCDIR)/java/lang/CharacterData02.java \
    $(GENSRCDIR)/java/lang/CharacterData0E.java \
    $(GENSRCDIR)/java/lang/CharacterDataLatin1.java \
    $(GENSRCDIR)/java/lang/CharacterDataUndefined.java \
    $(GENSRCDIR)/java/lang/CharacterDataPrivateUse.java

rbcontrolsrc: $(GENSRCDIR)/sun/util/CoreResourceBundleControl.java

#
# Add to the default C file search paths
#
vpath %.c $(PLATFORM_SRC)/native/java/lang:$(SHARE_SRC)/native/java/lang
vpath %.c $(SHARE_SRC)/native/java/lang/ref
vpath %.c $(SHARE_SRC)/native/java/lang/reflect
vpath %.c $(SHARE_SRC)/native/java/io
vpath %.c $(PLATFORM_SRC)/native/java/io
vpath %.c $(SHARE_SRC)/native/java/nio
vpath %.c $(SHARE_SRC)/native/java/security
vpath %.c $(SHARE_SRC)/native/common
vpath %.c $(SHARE_SRC)/native/sun/misc
vpath %.c $(SHARE_SRC)/native/sun/reflect
vpath %.c $(SHARE_SRC)/native/java/sql
vpath %.c $(SHARE_SRC)/native/java/util
vpath %.c $(SHARE_SRC)/native/java/util/concurrent/atomic
vpath %.c $(PLATFORM_SRC)/native/common
vpath %.c $(PLATFORM_SRC)/native/java/util
vpath %.c $(PLATFORM_SRC)/native/sun/security/provider
vpath %.c $(PLATFORM_SRC)/native/sun/io

#
# Includes the fdlibm header file.
#
OTHER_INCLUDES += -I$(SHARE_SRC)/native/java/lang/fdlibm/include

#
# Also include the io_util.h header file.
#
OTHER_INCLUDES += -I$(SHARE_SRC)/native/java/io
OTHER_INCLUDES += -I$(PLATFORM_SRC)/native/java/io

#
# Also include the TimeZone_md.h header file.
#
OTHER_INCLUDES += -I$(PLATFORM_SRC)/native/java/util

#
# Is the altzone extern documented in ctime(3C) available?
#
ifneq ($(PLATFORM), windows)
HAVE_ALTZONE=true
endif

ifeq ($(HAVE_ALTZONE),true)
OTHER_CPPFLAGS += -DHAVE_ALTZONE
endif

#
# System.c needs RELEASE.
# java_props_md.c needs ARCH on non-Solaris platforms.
#
OTHER_CPPFLAGS += $(VERSION_DEFINES)

#
# Include name of LIBARCH
#
OTHER_CPPFLAGS += -DARCHPROPNAME='"$(ARCHPROP)"'

#
# Is the key in the call nl_langinfo(CODESET) called something other than
# CODESET?
#
ifdef ALT_CODESET_KEY
OTHER_CPPFLAGS += -DALT_CODESET_KEY=$(ALT_CODESET_KEY)
endif

#
# Install .lib file.
#
INSTALL_DOT_LIB = true

#
# What to link?
#
ifeq ($(PLATFORM),windows)
OTHER_LDLIBS += $(JVMLIB) -libpath:$(OBJDIR)/../../../fdlibm/$(OBJDIRNAME) fdlibm.lib \
		       -libpath:$(OBJDIR)/../../../verify/$(OBJDIRNAME) verify.lib
else
LIBSOCKET = -lsocket
OTHER_LDLIBS += $(JVMLIB) -lverify $(LIBSOCKET) -lnsl -ldl \
		-L$(OBJDIR)/../../../fdlibm/$(OBJDIRNAME) -lfdlibm.$(ARCH)
endif

#
# Rules.
#
ifeq ($(PLATFORM), solaris)
  ifneq ($(ARCH), amd64)
    FILES_reorder += reorder-$(ARCH)
  endif
endif

SUBDIRS = reflect
all build:
	$(SUBDIRS-loop)
clean clobber::
	$(SUBDIRS-loop)

include $(BUILDDIR)/common/Mapfile-vers.gmk
include $(BUILDDIR)/common/Library.gmk

ifeq ($(PLATFORM), solaris)
  ifeq ($(CC_VER), 5.8)
    ifndef REMOVE_ALL_WORKAROUNDS
      CFLAGS_OPT/ObjectInputStream.o = -xO3 \
	  $(warning  "WARNING: Using workaround for SS11 bug 6346242, on $@")
    endif
  endif
endif

ifneq ($(PLATFORM),windows)

# UNIXProcess.java is different for solaris and linux. We need to copy
# the correct UNIXProcess.java over to $(GENSRCDIR)/java/lang/.

$(GENSRCDIR)/java/lang/UNIXProcess.java: \
    $(PLATFORM_SRC)/classes/java/lang/UNIXProcess.java.$(PLATFORM)
	$(install-file)

clean:: 
	$(RM) $(GENSRCDIR)/java/lang/UNIXProcess.java

endif


#
# Because we can't link against ourselves!
#
JAVALIB =

#
# Bootstrap stage.
#
JAVAC_CMD = $(JAVAC_BOOT_CMD)
JAVAH_CMD = $(JAVAH_BOOT_CMD)

#
# Has been converted to the JNI: generate JNI-style header files
#
JAVAHFLAGS += -jni

#
# Special rules.
#
clean:: 
	$(RM) -r $(CLASSHDRDIR)

clobber:: 
	$(RM) -r $(CLASSBINDIR)/java/io $(CLASSBINDIR)/java/lang \
		$(CLASSBINDIR)/java/security $(CLASSBINDIR)/java/util \
		$(CLASSBINDIR)/sun/misc

#
# Additional rule from sun/net/GNUmakefile to copy content-type.properties
# file so that the MimeTable class won't complain while bootstrapping...
#
PROPS = content-types.properties

#
# Rule to copy calendars.properties file.
#
CAL_PROPS = calendars.properties

#
# Rule to copy tzmappings file on Windows
#
ifeq ($(PLATFORM), windows)
TZMAP = $(LIBDIR)/tzmappings
TZMAPFILE = $(PLATFORM_SRC)/lib/tzmappings

$(TZMAP): $(TZMAPFILE)
	$(install-file)
	$(CHMOD) 444 $@
endif

build: $(LIBDIR)/$(PROPS) $(LIBDIR)/$(CAL_PROPS) $(TZMAP)

$(LIBDIR)/$(PROPS): $(PLATFORM_SRC)/lib/$(PROPS)
	$(install-file)

$(LIBDIR)/$(CAL_PROPS): $(SHARE_SRC)/lib/$(CAL_PROPS)
	$(install-file)

clean:: 
	$(RM) -r $(LIBDIR)/$(PROPS) $(TZMAP)

#
# Rules to create java/util/CurrencyData.java
#

GENCUROUT = $(TEMPDIR)/GenerateCurrencyData

$(GENSRCDIR)/java/util/CurrencyData.java: \
  $(BUILDDIR)/tools/GenerateCurrencyData/GenerateCurrencyData.java \
  $(SHARE_SRC)/classes/java/util/CurrencyData.properties
	@$(prep-target)
	$(RM) -r $(GENCUROUT)
	$(MKDIR) -p $(GENCUROUT)
	$(JAVAC_BOOT) -d $(GENCUROUT) \
		$(BUILDDIR)/tools/GenerateCurrencyData/GenerateCurrencyData.java
	@$(RM) $@.temp
	$(JAVA_BOOT) -classpath $(GENCUROUT) GenerateCurrencyData \
		-o $@.temp \
		< $(SHARE_SRC)/classes/java/util/CurrencyData.properties
	$(MV) $@.temp $@

clean:: 
	$(RM) $(GENSRCDIR)/java/util/CurrencyData.java
	$(RM) -r $(GENCUROUT)


#
# Rules to create $(GENSRCDIR)/sun/lang/CharacterData*.java 
#
GENCHARSRC = $(BUILDDIR)/tools/GenerateCharacter
GENCHAROUT = $(TEMPDIR)/GenerateCharacter

$(GENSRCDIR)/java/lang/CharacterDataLatin1.java \
  $(GENSRCDIR)/java/lang/CharacterData00.java \
  $(GENSRCDIR)/java/lang/CharacterData01.java \
  $(GENSRCDIR)/java/lang/CharacterData02.java \
  $(GENSRCDIR)/java/lang/CharacterData0E.java: \
    $(wildcard $(GENCHARSRC)/*) $(GENCHARSRC)/GenerateCharacter.java
	@$(prep-target)
	$(RM) -r $(GENCHAROUT)
	$(MKDIR) -p $(GENCHAROUT)
	$(JAVAC_BOOT) -d $(GENCHAROUT) \
		-sourcepath $(GENCHARSRC) $(GENCHARSRC)/GenerateCharacter.java
	$(JAVA_BOOT) -classpath $(GENCHAROUT) GenerateCharacter \
		-template $(GENCHARSRC)/CharacterDataLatin1.java.template \
		-spec $(GENCHARSRC)/UnicodeData.txt \
		-specialcasing $(GENCHARSRC)/SpecialCasing.txt \
		-o $(GENSRCDIR)/java/lang/CharacterDataLatin1.java -string \
		-usecharforbyte -latin1 8
	$(JAVA_BOOT) -classpath $(GENCHAROUT) GenerateCharacter -plane 0 \
		-template $(GENCHARSRC)/CharacterData00.java.template \
		-spec $(GENCHARSRC)/UnicodeData.txt \
		-specialcasing $(GENCHARSRC)/SpecialCasing.txt \
		-o $(GENSRCDIR)/java/lang/CharacterData00.java -string \
		-usecharforbyte 11 4 1
	$(JAVA_BOOT) -classpath $(GENCHAROUT) GenerateCharacter -plane 1 \
		-template $(GENCHARSRC)/CharacterData01.java.template \
		-spec $(GENCHARSRC)/UnicodeData.txt \
		-specialcasing $(GENCHARSRC)/SpecialCasing.txt \
		-o $(GENSRCDIR)/java/lang/CharacterData01.java -string \
		-usecharforbyte  11 4 1
	$(JAVA_BOOT) -classpath $(GENCHAROUT) GenerateCharacter -plane 2 \
		-template $(GENCHARSRC)/CharacterData02.java.template \
		-spec $(GENCHARSRC)/UnicodeData.txt \
		-specialcasing $(GENCHARSRC)/SpecialCasing.txt \
		-o $(GENSRCDIR)/java/lang/CharacterData02.java -string \
		-usecharforbyte 11 4 1
	$(JAVA_BOOT) -classpath $(GENCHAROUT) GenerateCharacter -plane 14 \
		-template $(GENCHARSRC)/CharacterData0E.java.template \
		-spec $(GENCHARSRC)/UnicodeData.txt \
		-specialcasing $(GENCHARSRC)/SpecialCasing.txt \
		-o $(GENSRCDIR)/java/lang/CharacterData0E.java -string \
		-usecharforbyte 11 4 1

$(GENSRCDIR)/java/lang/%.java : $(GENCHARSRC)/%.java.template
	$(install-file)

clean::
	$(RM) $(GENSRCDIR)/java/lang/CharacterDataLatin1.java 
	$(RM) $(GENSRCDIR)/java/lang/CharacterData00.java
	$(RM) $(GENSRCDIR)/java/lang/CharacterData01.java 
	$(RM) $(GENSRCDIR)/java/lang/CharacterData02.java
	$(RM) $(GENSRCDIR)/java/lang/CharacterData0E.java
	$(RM) $(GENSRCDIR)/java/lang/CharacterDataUndefined.java 
	$(RM) $(GENSRCDIR)/java/lang/CharacterDataPrivateUse.java
	$(RM) -r $(GENCHAROUT)

#
# End of rules to create $(GENSRCDIR)/java/lang/CharacterDataXX.java
#

#
# Rule to precompile CoreResourceBundleControl.java 
#
LOCALES_GEN_SH = localelist.sh

$(GENSRCDIR)/sun/util/CoreResourceBundleControl.java: \
	$(SHARE_SRC)/classes/sun/util/CoreResourceBundleControl-XLocales.java $(LOCALES_GEN_SH)
	@$(prep-target) 
	NAWK=$(NAWK) SED=$(SED) $(SH) $(LOCALES_GEN_SH) "$(J2RE_NONEXIST_LOCALES)" \
		$< $@ 
clean::
	$(RM) $(GENSRCDIR)/sun/util/CoreResourceBundleControl.java


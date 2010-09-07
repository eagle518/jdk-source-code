#
# @(#)Makefile	1.46 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
PACKAGE = javax.sound
LIBRARY = jsound
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

# include defines for sound
include SoundDefs.gmk

# support for different mixer provider files on different platforms
#MXSP = javax.sound.sampled.spi.MixerProvider
# Mixer Service Provider Additional path
#MXSPP_ADD =

#
# Add use of mapfile
#
FILES_m = mapfile-vers
include $(BUILDDIR)/common/Mapfile-vers.gmk

#
# Files
#
include FILES_c.gmk

#
# Files that just need cp.
#
SERVICEDIR = $(CLASSBINDIR)/META-INF/services

FILES_copy = \
	$(SERVICEDIR)/javax.sound.midi.spi.MidiDeviceProvider \
	$(SERVICEDIR)/javax.sound.midi.spi.MidiFileWriter \
	$(SERVICEDIR)/javax.sound.midi.spi.MidiFileReader \
	$(SERVICEDIR)/javax.sound.midi.spi.SoundbankReader \
	$(SERVICEDIR)/javax.sound.sampled.spi.AudioFileWriter \
	$(SERVICEDIR)/javax.sound.sampled.spi.AudioFileReader \
	$(SERVICEDIR)/javax.sound.sampled.spi.FormatConversionProvider \
	$(SERVICEDIR)/javax.sound.sampled.spi.MixerProvider \
	$(LIBDIR)/audio/soundbank.gm \
	$(LIBDIR)/sound.properties

FILES_mkdirs = \
	$(CLASSBINDIR)/META-INF \
	$(CLASSBINDIR)/META-INF/services \
	$(LIBDIR)/audio

FILES_copydirs = \
	$(CLASSBINDIR) \
	$(LIBDIR) \
	$(FILES_mkdirs)

FILES_c += $(FILES_engine) $(FILES_$(PLATFORM))

#
# system dependent flags
#
ifeq ($(PLATFORM), windows)
  CPPFLAGS += 	-DUSE_DIRECTSOUND=0 \
		-DUSE_EXTERNAL_SYNTH=TRUE \
		-DUSE_PLATFORM_MIDI_OUT=TRUE \
		-DUSE_PLATFORM_MIDI_IN=TRUE \
		-DUSE_PORTS=TRUE
  # Windows always provides MIDI and ports
  INCLUDE_MIDI  = TRUE
  INCLUDE_PORTS = TRUE

  # for now, only compile DirectSound for i586
  ifeq ($(ARCH), i586)
    SUBDIRS += jsoundds
    EXTRA_SOUND_JNI_LIBS += jsoundds
    #MXSPP_ADD = $(PLATFORM)-$(ARCH)/
  else
    # otherwise, build with empty direct audio
    INCLUDE_DAUDIO = TRUE
  endif
endif # PLATFORM win32

ifeq ($(PLATFORM), linux)
  ifneq ($(ARCH), ia64)
    # ALSA handles directaudio, ports, and MIDI
    SUBDIRS += jsoundalsa
    EXTRA_SOUND_JNI_LIBS += jsoundalsa
    #MXSPP_ADD = $(PLATFORM)-$(ARCH)/
  else
    # build with empty MIDI i/o
    INCLUDE_MIDI = TRUE
    # build with empty ports
    INCLUDE_PORTS = TRUE
    # build with empty direct audio
    INCLUDE_DAUDIO = TRUE
  endif
endif # PLATFORM linux

ifeq ($(PLATFORM), solaris)
  ifneq ($(ARCH), amd64)
    # build with ports and direct audio
    CPPFLAGS += -DUSE_PORTS=TRUE 	\
                -DUSE_DAUDIO=TRUE

    INCLUDE_PORTS = TRUE
    INCLUDE_DAUDIO = TRUE
    INCLUDE_MIDI = TRUE
  else
    CPPFLAGS += -DUSE_DAUDIO=TRUE

    # build with empty MIDI i/o
    INCLUDE_MIDI = TRUE
    # build with empty ports
    INCLUDE_PORTS = TRUE
    # build with empty direct audio
    INCLUDE_DAUDIO = TRUE
  endif
endif # PLATFORM solaris

# for dynamic inclusion of extra sound libs: these
# JNI libs will be loaded from Platform.java
CPPFLAGS += -DEXTRA_SOUND_JNI_LIBS="\"$(EXTRA_SOUND_JNI_LIBS)\""

# integrate MIDI i/o in jsound lib
ifeq ($(INCLUDE_MIDI),TRUE)
  FILES_c += $(MIDIFILES_c)
  FILES_export += $(MIDIFILES_export)
endif

# integrate port files in jsound lib
ifeq ($(INCLUDE_PORTS),TRUE)
  FILES_c += $(PORTFILES_c)
  FILES_export += $(PORTFILES_export)
endif

# integrate port files in jsound lib
ifeq ($(INCLUDE_DAUDIO),TRUE)
  FILES_c += $(DAUDIOFILES_c)
  FILES_export += $(DAUDIOFILES_export)
endif

#
# Include rules
#
include $(BUILDDIR)/common/Library.gmk

#
# Rules that copy files.
#

build: copy-files
	$(SUBDIRS-loop)

copy-files: $(FILES_copy)

#$(SERVICEDIR)/$(MXSP): $(SHARE_SRC)/classes/com/sun/media/sound/services/$(MXSPP_ADD)$(MXSP)
#	$(install-file)

$(SERVICEDIR)/%: $(SHARE_SRC)/classes/com/sun/media/sound/services/%
	$(install-file)

$(LIBDIR)/audio/%: $(SHARE_SRC)/lib/audio/%
	$(install-file)

$(LIBDIR)/%: $(SHARE_SRC)/lib/%
	$(install-file)

#
# Since we build more than the javax.sound (as stated in the
# PACKAGE variable), we need to nuke the extra packages explicitly.
#
clean clobber::
	$(SUBDIRS-loop)

clean clobber::
	$(RM) -r $(CLASSBINDIR)/com/sun/media/sound $(LIBDIR)/audio
	$(RM) $(FILES_copy)

#
# Declare extra phony targets.
#
.PHONY: copy-files

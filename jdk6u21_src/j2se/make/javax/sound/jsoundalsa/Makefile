#
# @(#)Makefile	1.12 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../../..
PACKAGE = javax.sound
LIBRARY = jsoundalsa
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

# this Makefile compiles the native files for alsa support in java sound

# include defines for sound
include ../SoundDefs.gmk

#
# Use mapfile
#
FILES_m = mapfile-vers
include $(BUILDDIR)/common/Mapfile-vers.gmk

#
# Files
#

FILES_c = \
	$(CTARGDIR)Utilities.c \
	$(DAUDIOFILES_c) \
	$(MIDIFILES_c) \
	$(PORTFILES_c)

# platform dependent files
FILES_c += \
	$(CTARGDIR)PLATFORM_API_LinuxOS_ALSA_CommonUtils.c   \
	$(CTARGDIR)PLATFORM_API_LinuxOS_ALSA_PCM.c     \
	$(CTARGDIR)PLATFORM_API_LinuxOS_ALSA_PCMUtils.c   \
	$(CTARGDIR)PLATFORM_API_LinuxOS_ALSA_MidiIn.c  \
	$(CTARGDIR)PLATFORM_API_LinuxOS_ALSA_MidiOut.c \
	$(CTARGDIR)PLATFORM_API_LinuxOS_ALSA_MidiUtils.c \
	$(CTARGDIR)PLATFORM_API_LinuxOS_ALSA_Ports.c

FILES_export = \
	$(DAUDIOFILES_export) \
	$(MIDIFILES_export) \
	$(PORTFILES_export)

LDFLAGS += -lasound

CPPFLAGS += \
	-DUSE_DAUDIO=TRUE \
	-DUSE_PORTS=TRUE  \
	-DUSE_PLATFORM_MIDI_OUT=TRUE \
	-DUSE_PLATFORM_MIDI_IN=TRUE  \
	-DUSE_EXTERNAL_SYNTH=TRUE


#
# Include rules
#
include $(BUILDDIR)/common/Library.gmk




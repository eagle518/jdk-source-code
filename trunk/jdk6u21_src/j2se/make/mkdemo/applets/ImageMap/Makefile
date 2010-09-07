#
# @(#)Makefile	1.9 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Build applet demo.
#

BUILDDIR = ../../..
DEMONAME = ImageMap
include $(BUILDDIR)/common/Defs.gmk

DEMO_java  = $(DEMODST)/AniArea.java \
             $(DEMODST)/ButtonFilter.java \
             $(DEMODST)/ClickArea.java \
             $(DEMODST)/DelayedSoundArea.java \
             $(DEMODST)/HighlightArea.java \
             $(DEMODST)/HighlightFilter.java \
             $(DEMODST)/HrefButtonArea.java \
             $(DEMODST)/ImageMap.java \
             $(DEMODST)/ImageMapArea.java \
             $(DEMODST)/LinkArea.java \
             $(DEMODST)/NameArea.java \
             $(DEMODST)/RoundButtonFilter.java \
             $(DEMODST)/RoundHrefButtonArea.java \
             $(DEMODST)/SoundArea.java \
             $(DEMODST)/TickerArea.java

DEMO_dirs  = $(DEMODST) \
             $(DEMODST)/audio \
             $(DEMODST)/images

DEMO_other = $(DEMODST)/example1.html \
             $(DEMODST)/sources.html \
             $(DEMODST)/imagemapcheat.html \
             $(DEMODST)/audio/chirp1.au \
             $(DEMODST)/audio/hi.au \
             $(DEMODST)/images/jim.graham.gif

include ../DemoApplets.gmk


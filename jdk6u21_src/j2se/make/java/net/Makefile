#
# @(#)Makefile	1.61 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
PACKAGE = java.net
LIBRARY = net
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
include FILES_c.gmk

AUTO_FILES_JAVA_DIRS = java/net
FILES_java += java/net/URL.java

ifeq ($(PLATFORM), windows)
    # Windows 9x module only needed on 32-bit build
    ifeq ($(ARCH_DATA_MODEL), 32)
	FILES_c += NetworkInterface_win9x.c
    endif
    FILES_c += NTLMAuthSequence.c
    FILES_c += NetworkInterface_winXP.c
endif

FILES_export = \
    java/net/Socket.java \
    java/net/SocketOptions.java \
    java/net/SocketImpl.java \
    java/net/PlainSocketImpl.java \
    java/net/ServerSocket.java \
    java/net/InetAddress.java \
    java/net/Inet4Address.java \
    java/net/Inet6Address.java \
    java/net/InetAddressImpl.java \
    java/net/Inet4AddressImpl.java \
    java/net/Inet6AddressImpl.java \
    java/net/NetworkInterface.java \
    java/net/SocketInputStream.java \
    java/net/SocketOutputStream.java \
    java/net/DatagramPacket.java \
    java/net/DatagramSocket.java \
    java/net/DatagramSocketImpl.java \
    java/net/PlainDatagramSocketImpl.java \
    java/net/MulticastSocket.java \
    java/net/UnknownHostException.java \
    java/net/ProtocolException.java \
    sun/net/spi/DefaultProxySelector.java

#
# Find platform specific native code
#
vpath %.c $(PLATFORM_SRC)/native/sun/net/dns $(PLATFORM_SRC)/native/sun/net/www/protocol/http $(PLATFORM_SRC)/native/sun/net/spi

#
# Use JNI for generating header files
#
JAVAHFLAGS += -jni

#
# Include rules
#
include $(BUILDDIR)/common/Mapfile-vers.gmk

include $(BUILDDIR)/common/Library.gmk

ifeq ($(PLATFORM), windows)
  OTHER_LDLIBS = ws2_32.lib $(JVMLIB)
  # Will not compile at warning level 3 if warnings are fatal
  COMPILER_WARNINGS_FATAL=false
else
  LIBSOCKET = -lsocket
  OTHER_LDLIBS = $(LIBSOCKET) -lnsl -ldl $(JVMLIB)
endif
ifeq ($(PLATFORM), linux)
  OTHER_LDLIBS += -lpthread
endif

CLASSES.export += java.lang.Integer java.io.FileDescriptor java.net.InetAddressImplFactory java.net.Inet4AddressImpl java.net.Inet6AddressImpl

#
# Resources
#
LOCALE_SET_DEFINITION = j2re

properties: $(LIBDIR) $(LIBDIR)/net.properties

$(LIBDIR)/net.properties: $(SHARE_SRC)/lib/net.properties
	@$(RM) $@
	$(CP) $< $@

build: properties


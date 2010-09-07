#######################################################################
#
# Makefile for building wsgen
#

BUILDDIR = ../..
PACKAGE = com.sun.tools.internal.ws
PRODUCT = sun
PROGRAM = wsgen
include $(BUILDDIR)/common/Defs.gmk

#
# This program must contain a manifest that defines the execution level
# needed to follow standard Vista User Access Control Guidelines
# This must be set before Program.gmk is included
#
BUILD_MANIFEST=true

#
# Files. (If java sources need to be compiled)
#
AUTO_FILES_JAVA_DIRS = com/sun/tools/internal/ws

#
# Resources (only if you have them)
#
include $(BUILDDIR)/common/Resources.gmk

#
# Rules.
#


JAVA_ARGS = "{ \"-J-ms8m\", \"com.sun.tools.internal.ws.WsGen\" }"


include $(BUILDDIR)/common/Program.gmk

#######################################################################



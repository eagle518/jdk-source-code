#
# @(#)Makefile	1.13 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.13 10/03/23
#

BUILDDIR = ../../../..
PACKAGE = com.sun.security.auth
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
include FILES_java.gmk

#
# Resources
#
LOCALE_SET_DEFINITION = j2re
RESOURCE_BUNDLES_JAVA = $(TARGDIR)/sun/security/util/AuthResources.java
include $(BUILDDIR)/common/Resources.gmk

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

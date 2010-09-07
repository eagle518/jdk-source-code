#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.9 10/03/23
#

#
# @(#)Makefile	1.11 01/10/09
#
# Makefile for building keytab utility.
#

BUILDDIR = ../../..
PACKAGE = sun.tools.security
PRODUCT = sun
PROGRAM = ktab
include $(BUILDDIR)/common/Defs.gmk

#
# This program must contain a manifest that defines the execution level 
# needed to follow standard Vista User Access Control Guidelines
# This must be set before Program.gmk is included
# 
BUILD_MANIFEST=true

#
# Class to launch tools with.
#
JAVA_ARGS = "{ \"-J-ms8m\", \"sun.security.krb5.internal.tools.Ktab\" }"

#
# Rules.
#
include $(BUILDDIR)/common/Program.gmk

# @(#)Makefile	1.11 03/12/23 16:35:38
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

!ifdef ADLC_ONLY
SUBDIRS=generated 
!else
SUBDIRS=generated release debug
!endif

!include ../local.make
!include local.make
!include $(WorkSpace)/build/windows/makefiles/shared.make


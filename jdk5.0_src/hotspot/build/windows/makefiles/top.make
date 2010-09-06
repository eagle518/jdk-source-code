# @(#)top.make	1.19 03/12/23 16:35:36
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

!include local.make

!if "$(Variant)" == "docs"
SUBDIRS=docs
!else
SUBDIRS=bin vm
!endif

!include $(WorkSpace)/build/windows/makefiles/shared.make


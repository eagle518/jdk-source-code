# @(#)debug.make	1.14 04/04/12 10:49:17
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

!include compile.make

!if "$(ARCH)" == "amd64"
CPP_FLAGS=$(CPP_FLAGS) /Zi /Od /MD /D "_DEBUG"
!else
CPP_FLAGS=$(CPP_FLAGS) /Zi /Od /MDd /D "_DEBUG"
!endif

LINK_FLAGS=$(LINK_FLAGS) /debug /export:opnames

RC_FLAGS=$(RC_FLAGS) /D "_DEBUG"

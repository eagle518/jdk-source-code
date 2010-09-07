#
# Copyright (c) 1997, 2008, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#  
#

!include local.make

!ifdef ADLC_ONLY
SUBDIRS=generated
!else
SUBDIRS=generated $(BUILD_FLAVOR)
!endif

!include $(WorkSpace)/make/windows/makefiles/shared.make


#
# Copyright (c) 2005, 2008, Oracle and/or its affiliates. All rights reserved.
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

HS_INTERNAL_NAME=jvm
HS_FNAME=$(HS_INTERNAL_NAME).dll
AOUT=$(HS_FNAME)
SAWINDBG=sawindbg.dll
GENERATED=../generated

default:: _build_pch_file.obj $(AOUT) checkAndBuildSA

!include ../local.make
!include compile.make

CPP_FLAGS=$(CPP_FLAGS) $(FASTDEBUG_OPT_OPTION)

!include $(WorkSpace)/make/windows/makefiles/vm.make
!include local.make

!include $(GENERATED)/Dependencies

HS_BUILD_ID=$(HS_BUILD_VER)-fastdebug

# Force resources to be rebuilt every time
$(Res_Files): FORCE

$(AOUT): $(Res_Files) $(Obj_Files)
	sh $(WorkSpace)/make/windows/build_vm_def.sh
	$(LINK) @<<
  $(LINK_FLAGS) /out:$@ /implib:$*.lib /def:vm.def $(Obj_Files) $(Res_Files)
<<
!if "$(MT)" != ""
# The previous link command created a .manifest file that we want to
# insert into the linked artifact so we do not need to track it
# separately.  Use ";#2" for .dll and ";#1" for .exe:
	$(MT) /manifest $@.manifest /outputresource:$@;#2
!endif


!include $(WorkSpace)/make/windows/makefiles/shared.make
!include $(WorkSpace)/make/windows/makefiles/sa.make

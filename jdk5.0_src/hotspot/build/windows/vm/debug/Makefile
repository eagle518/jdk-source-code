# @(#)Makefile	1.16 04/04/12 10:49:54
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

HS_INTERNAL_NAME=jvm_g
HS_FNAME=$(HS_INTERNAL_NAME).dll
AOUT=../../bin/$(HS_FNAME)
SAWINDBG=../../bin/sawindbg_g.dll
GENERATED=../generated

default:: _build_pch_file.obj $(AOUT) checkAndBuildSA

!include ../../local.make
!include $(WorkSpace)/build/windows/makefiles/debug.make
!include $(WorkSpace)/build/windows/makefiles/vm.make
!include ../local.make
!include local.make

!include ../generated/Dependencies

HS_BUILD_ID=$(HS_BUILD_ID)-debug

# Force resources to be rebuilt every time
$(Res_Files): FORCE

$(AOUT): $(Res_Files) $(Obj_Files)
	sh $(WorkSpace)/build/windows/build_vm_def.sh
	$(LINK) @<<
  $(LINK_FLAGS) /out:$@ /implib:$*.lib /def:vm.def $(Obj_Files) $(Res_Files)
<<

# These do not need to be optimized (don't run a lot of code) and it
# will be useful to have the assertion checks in place

WINDBG_INCLUDE=$(WindbgHome)\sdk\inc

!if "$(ARCH)" == "ia64"
SA_CFLAGS = /nologo /MD /W3 /GX /Od /D "WIN32" /D "WIN64" /D "_WINDOWS" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
WINDBG_LIB=$(WindbgHome)\sdk\lib\ia64
SA_LFLAGS = $(WINDBG_LIB)\dbgeng.lib /nologo /subsystem:console /debug /machine:IA64
!else
SA_CFLAGS = /nologo /MD /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_WINDOWS" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
WINDBG_LIB = $(WindbgHome)\sdk\lib\i386
SA_LFLAGS  = $(WINDBG_LIB)\dbgeng.lib  /nologo /subsystem:console /debug /machine:I386
!endif

AGENT_DIR = $(WorkSpace)/agent
SASRCFILE = $(AGENT_DIR)/src/os/win32/windbg/sawindbg.cpp

# if WindbgHome is not defined or if BootStrapDir is not defined or 
# if $(AGENT_DIR) does not exist, we don't build SA.

checkAndBuildSA:
!if defined(WindbgHome) && defined(BootStrapDir)
	@if exist $(AGENT_DIR) $(MAKE) -f $(WorkSpace)\build\windows\vm\debug\Makefile WorkSpace=$(WorkSpace) BootStrapDir=$(BootStrapDir) $(SAWINDBG)
!endif

$(SAWINDBG): $(SASRCFILE) 
        $(CPP) @<<
  /I$(BootStrapDir)/include /I$(BootStrapDir)/include/win32
  /I$(WINDBG_INCLUDE) /I$(GENERATED) $(SA_CFLAGS)
  $(SASRCFILE) 
  /out:sawindbg.obj
<<
        $(LINK) /out:$@ /DLL sawindbg.obj $(WINDBG_LIB)/dbgeng.lib $(SA_LFLAGS) 

!include $(WorkSpace)/build/windows/makefiles/shared.make

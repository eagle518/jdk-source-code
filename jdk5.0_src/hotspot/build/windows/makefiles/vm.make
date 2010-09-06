# @(#)vm.make	1.49 04/05/20 13:10:42
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# Resource file containing VERSIONINFO
Res_Files=.\version.res

!ifdef RELEASE 
!ifdef DEVELOP
CPP_FLAGS=$(CPP_FLAGS) /D "DEBUG"
!else
CPP_FLAGS=$(CPP_FLAGS) /D "PRODUCT"
!endif
!else
CPP_FLAGS=$(CPP_FLAGS) /D "ASSERT"
!endif

!if "$(Variant)" == "core"
# No need to define anything, CORE is defined as !COMPILER1 && !COMPILER2
!endif

!if "$(Variant)" == "compiler1"
CPP_FLAGS=$(CPP_FLAGS) /D "COMPILER1"
!endif

!if "$(Variant)" == "compiler2"
CPP_FLAGS=$(CPP_FLAGS) /D "COMPILER2"
!endif

!if "$(BuildID)" != ""
CPP_FLAGS=$(CPP_FLAGS) /D "HOTSPOT_BUILD_VERSION=\"$(BuildID)\""
!endif

!if "$(BuildUser)" != ""
CPP_FLAGS=$(CPP_FLAGS) /D "HOTSPOT_BUILD_USER=\"$(BuildUser)\""
!endif

# # do not build with JVMPI for RC1
# CPP_FLAGS=$(CPP_FLAGS) /D "JVMPI" /D "WIN32" /D "_WINDOWS" $(CPP_INCLUDE_DIRS)
CPP_FLAGS=$(CPP_FLAGS) /D "WIN32" /D "_WINDOWS" $(CPP_INCLUDE_DIRS)

# Must specify this for sharedRuntimeTrig.cpp
CPP_FLAGS=$(CPP_FLAGS) /D "VM_LITTLE_ENDIAN"

!if "$(ARCH)" == "ia64"
STACK_SIZE="/STACK:1048576,262144"
!else
STACK_SIZE=
!endif

!if "$(ARCH)" == "ia64"
# AsyncGetCallTrace is not supported on IA64 yet
AGCT_EXPORT=
!else
AGCT_EXPORT=/export:AsyncGetCallTrace
!endif

LINK_FLAGS=$(LINK_FLAGS) $(STACK_SIZE) /subsystem:windows /dll /base:0x8000000  \
  /export:JNI_GetDefaultJavaVMInitArgs /export:JNI_CreateJavaVM    \
  /export:JNI_GetCreatedJavaVMs /export:jio_snprintf               \
  /export:jio_printf /export:jio_fprintf                           \
  /export:jio_vfprintf /export:jio_vsnprintf $(AGCT_EXPORT)        \
  /export:JVM_GetThreadStateNames /export:JVM_GetThreadStateValues 

CPP_INCLUDE_DIRS=\
  /I "..\generated"                          \
  /I "..\generated\jvmtifiles"               \
  /I "$(WorkSpace)\src\share\vm\c1"          \
  /I "$(WorkSpace)\src\share\vm\compiler"    \
  /I "$(WorkSpace)\src\share\vm\code"        \
  /I "$(WorkSpace)\src\share\vm\interpreter" \
  /I "$(WorkSpace)\src\share\vm\ci"          \
  /I "$(WorkSpace)\src\share\vm\jvmci"       \
  /I "$(WorkSpace)\src\share\vm\lookup"      \
  /I "$(WorkSpace)\src\share\vm\gc_implementation\parallelScavenge"\
  /I "$(WorkSpace)\src\share\vm\gc_implementation\shared"\
  /I "$(WorkSpace)\src\share\vm\gc_interface"\
  /I "$(WorkSpace)\src\share\vm\asm"         \
  /I "$(WorkSpace)\src\share\vm\memory"      \
  /I "$(WorkSpace)\src\share\vm\oops"        \
  /I "$(WorkSpace)\src\share\vm\prims"       \
  /I "$(WorkSpace)\src\share\vm\runtime"     \
  /I "$(WorkSpace)\src\share\vm\services"    \
  /I "$(WorkSpace)\src\share\vm\utilities"   \
  /I "$(WorkSpace)\src\share\vm\libadt"      \
  /I "$(WorkSpace)\src\share\vm\opto"        \
  /I "$(WorkSpace)\src\os\win32\vm"          \
  /I "$(WorkSpace)\src\os_cpu\win32_$(ARCH)\vm" \
  /I "$(WorkSpace)\src\cpu\$(ARCH)\vm"

CPP_USE_PCH=/Fp"vm.pch" /Yu"incls/_precompiled.incl"

# Where to find the source code for the virtual machine
VM_PATH=../generated/incls
VM_PATH=$(VM_PATH);../generated/jvmtifiles
VM_PATH=$(VM_PATH);$(WorkSpace)/src/share/vm/c1
VM_PATH=$(VM_PATH);$(WorkSpace)/src/share/vm/compiler
VM_PATH=$(VM_PATH);$(WorkSpace)/src/share/vm/code
VM_PATH=$(VM_PATH);$(WorkSpace)/src/share/vm/interpreter
VM_PATH=$(VM_PATH);$(WorkSpace)/src/share/vm/ci
VM_PATH=$(VM_PATH);$(WorkSpace)/src/share/vm/jvmci
VM_PATH=$(VM_PATH);$(WorkSpace)/src/share/vm/lookup
VM_PATH=$(VM_PATH);$(WorkSpace)/src/share/vm/gc_implementation/parallelScavenge
VM_PATH=$(VM_PATH);$(WorkSpace)/src/share/vm/gc_implementation/shared
VM_PATH=$(VM_PATH);$(WorkSpace)/src/share/vm/gc_interface
VM_PATH=$(VM_PATH);$(WorkSpace)/src/share/vm/asm
VM_PATH=$(VM_PATH);$(WorkSpace)/src/share/vm/memory
VM_PATH=$(VM_PATH);$(WorkSpace)/src/share/vm/oops
VM_PATH=$(VM_PATH);$(WorkSpace)/src/share/vm/prims
VM_PATH=$(VM_PATH);$(WorkSpace)/src/share/vm/runtime
VM_PATH=$(VM_PATH);$(WorkSpace)/src/share/vm/services
VM_PATH=$(VM_PATH);$(WorkSpace)/src/share/vm/utilities
VM_PATH=$(VM_PATH);$(WorkSpace)/src/share/vm/libadt
VM_PATH=$(VM_PATH);$(WorkSpace)/src/os/win32/vm
VM_PATH=$(VM_PATH);$(WorkSpace)/src/os_cpu/win32_$(ARCH)/vm
VM_PATH=$(VM_PATH);$(WorkSpace)/src/cpu/$(ARCH)/vm
VM_PATH=$(VM_PATH);$(WorkSpace)/src/share/vm/opto

VM_PATH={$(VM_PATH)}

# Special case files not using precompiled header files.

c1_RInfo_$(ARCH).obj: $(WorkSpace)\src\cpu\$(ARCH)\vm\c1_RInfo_$(ARCH).cpp 
	 @$(CPP) $(CPP_FLAGS) /c $(WorkSpace)\src\cpu\$(ARCH)\vm\c1_RInfo_$(ARCH).cpp

os_win32.obj: $(WorkSpace)\src\os\win32\vm\os_win32.cpp
        @$(CPP) $(CPP_FLAGS) /c $(WorkSpace)\src\os\win32\vm\os_win32.cpp

os_win32_$(ARCH).obj: $(WorkSpace)\src\os_cpu\win32_$(ARCH)\vm\os_win32_$(ARCH).cpp
        @$(CPP) $(CPP_FLAGS) /c $(WorkSpace)\src\os_cpu\win32_$(ARCH)\vm\os_win32_$(ARCH).cpp

osThread_win32.obj: $(WorkSpace)\src\os\win32\vm\osThread_win32.cpp
        @$(CPP) $(CPP_FLAGS) /c $(WorkSpace)\src\os\win32\vm\osThread_win32.cpp

conditionVar_win32.obj: $(WorkSpace)\src\os\win32\vm\conditionVar_win32.cpp
        @$(CPP) $(CPP_FLAGS) /c $(WorkSpace)\src\os\win32\vm\conditionVar_win32.cpp

getThread_win32_$(ARCH).obj: $(WorkSpace)\src\os_cpu\win32_$(ARCH)\vm\getThread_win32_$(ARCH).cpp
        @$(CPP) $(CPP_FLAGS) /c $(WorkSpace)\src\os_cpu\win32_$(ARCH)\vm\getThread_win32_$(ARCH).cpp

opcodes.obj: $(WorkSpace)\src\share\vm\opto\opcodes.cpp
        @$(CPP) $(CPP_FLAGS) /c $(WorkSpace)\src\share\vm\opto\opcodes.cpp

# Default rules for the Virtual Machine
{$(WorkSpace)\src\share\vm\c1}.cpp.obj:
        @$(CPP) $(CPP_FLAGS) $(CPP_USE_PCH) /c $<

{$(WorkSpace)\src\share\vm\compiler}.cpp.obj:
        @$(CPP) $(CPP_FLAGS) $(CPP_USE_PCH) /c $<

{$(WorkSpace)\src\share\vm\code}.cpp.obj:
        @$(CPP) $(CPP_FLAGS) $(CPP_USE_PCH) /c $<

{$(WorkSpace)\src\share\vm\interpreter}.cpp.obj:
        @$(CPP) $(CPP_FLAGS) $(CPP_USE_PCH) /c $<

{$(WorkSpace)\src\share\vm\ci}.cpp.obj:
        @$(CPP) $(CPP_FLAGS) $(CPP_USE_PCH) /c $<

{$(WorkSpace)\src\share\vm\jvmci}.cpp.obj:
        @$(CPP) $(CPP_FLAGS) $(CPP_USE_PCH) /c $<

{$(WorkSpace)\src\share\vm\lookup}.cpp.obj:
        @$(CPP) $(CPP_FLAGS) $(CPP_USE_PCH) /c $<

{$(WorkSpace)\src\share\vm\gc_implementation\parallelScavenge}.cpp.obj:
        @$(CPP) $(CPP_FLAGS) $(CPP_USE_PCH) /c $<

{$(WorkSpace)\src\share\vm\gc_implementation\shared}.cpp.obj:
        @$(CPP) $(CPP_FLAGS) $(CPP_USE_PCH) /c $<

{$(WorkSpace)\src\share\vm\gc_interface}.cpp.obj:
        @$(CPP) $(CPP_FLAGS) $(CPP_USE_PCH) /c $<

{$(WorkSpace)\src\share\vm\asm}.cpp.obj:
        @$(CPP) $(CPP_FLAGS) $(CPP_USE_PCH) /c $<

{$(WorkSpace)\src\share\vm\memory}.cpp.obj:
        @$(CPP) $(CPP_FLAGS) $(CPP_USE_PCH) /c $<

{$(WorkSpace)\src\share\vm\oops}.cpp.obj:
        @$(CPP) $(CPP_FLAGS) $(CPP_USE_PCH) /c $<

{$(WorkSpace)\src\share\vm\prims}.cpp.obj:
        @$(CPP) $(CPP_FLAGS) $(CPP_USE_PCH) /c $<

{$(WorkSpace)\src\share\vm\runtime}.cpp.obj:
        @$(CPP) $(CPP_FLAGS) $(CPP_USE_PCH) /c $<

{$(WorkSpace)\src\share\vm\services}.cpp.obj:
        @$(CPP) $(CPP_FLAGS) $(CPP_USE_PCH) /c $<

{$(WorkSpace)\src\share\vm\utilities}.cpp.obj:
        @$(CPP) $(CPP_FLAGS) $(CPP_USE_PCH) /c $<

{$(WorkSpace)\src\share\vm\libadt}.cpp.obj:
        @$(CPP) $(CPP_FLAGS) $(CPP_USE_PCH) /c $<

{$(WorkSpace)\src\share\vm\opto}.cpp.obj:
        @$(CPP) $(CPP_FLAGS) $(CPP_USE_PCH) /c $<

{$(WorkSpace)\src\os\win32\vm}.cpp.obj:
        @$(CPP) $(CPP_FLAGS) $(CPP_USE_PCH) /c $<

# This guy should remain a single colon rule because
# otherwise we can't specify the output filename.
{$(WorkSpace)\src\os\win32\vm}.rc.res:
        @$(RC) $(RC_FLAGS) /fo"$@" $<

{$(WorkSpace)\src\cpu\$(ARCH)\vm}.cpp.obj:
        @$(CPP) $(CPP_FLAGS) $(CPP_USE_PCH) /c $<

{$(WorkSpace)\src\os_cpu\win32_$(ARCH)\vm}.cpp.obj:
        @$(CPP) $(CPP_FLAGS) $(CPP_USE_PCH) /c $<

{..\generated\incls}.cpp.obj:
        @$(CPP) $(CPP_FLAGS) $(CPP_USE_PCH) /c $<

{..\generated\jvmtifiles}.cpp.obj:
        @$(CPP) $(CPP_FLAGS) $(CPP_USE_PCH) /c $<

default::

_build_pch_file.obj:
        @echo #include "incls/_precompiled.incl" > ../generated/_build_pch_file.cpp
        @$(CPP) $(CPP_FLAGS) /Fp"vm.pch" /Yc"incls/_precompiled.incl" /c ../generated/_build_pch_file.cpp

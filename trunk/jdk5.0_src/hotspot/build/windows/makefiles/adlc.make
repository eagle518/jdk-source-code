# @(#)adlc.make	1.8 03/12/23 16:35:33
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# Rules for building adlc.exe

# Need exception handling support here
EXH_FLAGS=/GX

!ifdef ALT_ADLC_PATH
ADLC=$(ALT_ADLC_PATH)\adlc.exe
!else
ADLC=adlc
!endif

CPP_INCLUDE_DIRS=\
  /I "..\generated"                          \
  /I "$(WorkSpace)\src\share\vm\compiler"    \
  /I "$(WorkSpace)\src\share\vm\code"        \
  /I "$(WorkSpace)\src\share\vm\interpreter" \
  /I "$(WorkSpace)\src\share\vm\lookup"      \
  /I "$(WorkSpace)\src\share\vm\asm"         \
  /I "$(WorkSpace)\src\share\vm\memory"      \
  /I "$(WorkSpace)\src\share\vm\oops"        \
  /I "$(WorkSpace)\src\share\vm\prims"       \
  /I "$(WorkSpace)\src\share\vm\runtime"     \
  /I "$(WorkSpace)\src\share\vm\utilities"   \
  /I "$(WorkSpace)\src\share\vm\libadt"      \
  /I "$(WorkSpace)\src\share\vm\opto"        \
  /I "$(WorkSpace)\src\os\win32\vm"          \
  /I "$(WorkSpace)\src\cpu\$(ARCH)\vm"

# NOTE! If you add any files here, you must also update GENERATED_NAMES_IN_INCL
# and MakeDepsIDEOptions in makedeps.make. 
GENERATED_NAMES=\
  ad_$(ARCH).cpp \
  ad_$(ARCH).hpp \
  ad_$(ARCH)_clone.cpp \
  ad_$(ARCH)_expand.cpp \
  ad_$(ARCH)_format.cpp \
  ad_$(ARCH)_gen.cpp \
  ad_$(ARCH)_misc.cpp \
  ad_$(ARCH)_peephole.cpp \
  ad_$(ARCH)_pipeline.cpp \
  adGlobals_$(ARCH).hpp \
  dfa_$(ARCH).cpp

# NOTE! This must be kept in sync with GENERATED_NAMES
GENERATED_NAMES_IN_INCL=\
  incls/ad_$(ARCH).cpp \
  incls/ad_$(ARCH).hpp \
  incls/ad_$(ARCH)_clone.cpp \
  incls/ad_$(ARCH)_expand.cpp \
  incls/ad_$(ARCH)_format.cpp \
  incls/ad_$(ARCH)_gen.cpp \
  incls/ad_$(ARCH)_misc.cpp \
  incls/ad_$(ARCH)_peephole.cpp \
  incls/ad_$(ARCH)_pipeline.cpp \
  incls/adGlobals_$(ARCH).hpp \
  incls/dfa_$(ARCH).cpp

{$(WorkSpace)\src\share\vm\adlc}.cpp.obj:
        @$(CPP) $(CPP_FLAGS) $(EXH_FLAGS) $(CPP_INCLUDE_DIRS) /c $<

{$(WorkSpace)\src\share\vm\opto}.cpp.obj:
        @$(CPP) $(CPP_FLAGS) $(EXH_FLAGS) $(CPP_INCLUDE_DIRS) /c $<

adlc.exe: main.obj adlparse.obj archDesc.obj arena.obj dfa.obj dict2.obj filebuff.obj \
          forms.obj formsopt.obj formssel.obj opcodes.obj output_c.obj output_h.obj
	$(LINK) $(LINK_FLAGS) /subsystem:console /out:$@ $**

$(GENERATED_NAMES_IN_INCL): $(ARCH).ad adlc.exe includeDB.current 
	rm -f $(GENERATED_NAMES)
	$(ADLC) $(ARCH).ad
	mv $(GENERATED_NAMES) incls/

$(ARCH).ad: $(WorkSpace)/src/cpu/$(ARCH)/vm/$(ARCH).ad $(WorkSpace)/src/os_cpu/win32_$(ARCH)/vm/win32_$(ARCH).ad
	rm -f $(ARCH).ad
	cat $(WorkSpace)/src/cpu/$(ARCH)/vm/$(ARCH).ad  \
	    $(WorkSpace)/src/os_cpu/win32_$(ARCH)/vm/win32_$(ARCH).ad >$(ARCH).ad

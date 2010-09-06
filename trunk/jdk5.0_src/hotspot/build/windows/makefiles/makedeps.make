# @(#)makedeps.make	1.28 03/12/23 16:35:34
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

!include $(WorkSpace)/build/windows/makefiles/rules.make

# This is used externally by both batch and IDE builds, so can't
# reference any of the HOTSPOTWORKSPACE, HOTSPOTBUILDSPACE, or
# HOTSPOTBINDEST environment variables.
#
# NOTE: unfortunately the MakeDepsSources list must be kept
# synchronized between this and the Solaris version
# (build/solaris/makefiles/makedeps.make).

MakeDepsSources=\
        $(WorkSpace)\src\share\tools\MakeDeps\Database.java \
        $(WorkSpace)\src\share\tools\MakeDeps\DirectoryTree.java \
        $(WorkSpace)\src\share\tools\MakeDeps\DirectoryTreeNode.java \
        $(WorkSpace)\src\share\tools\MakeDeps\FileFormatException.java \
        $(WorkSpace)\src\share\tools\MakeDeps\FileList.java \
        $(WorkSpace)\src\share\tools\MakeDeps\FileName.java \
        $(WorkSpace)\src\share\tools\MakeDeps\Macro.java \
        $(WorkSpace)\src\share\tools\MakeDeps\MacroDefinitions.java \
        $(WorkSpace)\src\share\tools\MakeDeps\MakeDeps.java \
        $(WorkSpace)\src\share\tools\MakeDeps\MetroWerksMacPlatform.java \
        $(WorkSpace)\src\share\tools\MakeDeps\Platform.java \
        $(WorkSpace)\src\share\tools\MakeDeps\UnixPlatform.java \
        $(WorkSpace)\src\share\tools\MakeDeps\WinGammaPlatform.java

# This is only used internally
MakeDepsIncludesPRIVATE=\
        -relativeInclude src\share\vm\c1 \
        -relativeInclude src\share\vm\compiler \
        -relativeInclude src\share\vm\code \
        -relativeInclude src\share\vm\interpreter \
        -relativeInclude src\share\vm\ci \
        -relativeInclude src\share\vm\jvmci \
        -relativeInclude src\share\vm\lookup \
        -relativeInclude src\share\vm\gc_implementation\parallelScavenge \
        -relativeInclude src\share\vm\gc_implementation\shared \
        -relativeInclude src\share\vm\gc_interface \
        -relativeInclude src\share\vm\asm \
        -relativeInclude src\share\vm\memory \
        -relativeInclude src\share\vm\oops \
        -relativeInclude src\share\vm\prims \
        -relativeInclude src\share\vm\runtime \
        -relativeInclude src\share\vm\services \
        -relativeInclude src\share\vm\utilities \
        -relativeInclude src\share\vm\libadt \
        -relativeInclude src\share\vm\opto \
        -relativeInclude src\os\win32\vm \
        -relativeInclude src\os_cpu\win32_$(ARCH)\vm \
        -relativeInclude src\cpu\$(ARCH)\vm

# This is referenced externally by both the IDE and batch builds
MakeDepsOptions=

# This is used externally, but only by the IDE builds, so we can
# reference environment variables which aren't defined in the batch
# build process.

# Common options for the IDE builds for core, c1, and c2
MakeDepsIDEOptions=\
        -sourceBase $(HOTSPOTWORKSPACE) \
        -startAt src \
        -dspFileName $(HOTSPOTBUILDSPACE)\vm.dsp \
        -dllLoc $(HOTSPOTBINDEST) \
        -define ALIGN_STACK_FRAMES \
        -define VM_LITTLE_ENDIAN \
        -conditionalPerFileLine interpreter_abs.cpp "" " ADD CPP /Ze" \
        -perFileLine java.cpp " ADD CPP /Yc\"incls/_precompiled.incl\"" \
        -perFileLine os_win32.cpp " SUBTRACT CPP /YX /Yc /Yu" \
        -perFileLine os_win32_$(ARCH).cpp " SUBTRACT CPP /YX /Yc /Yu" \
        -perFileLine osThread_win32.cpp " SUBTRACT CPP /YX /Yc /Yu" \
        -perFileLine conditionVar_win32.cpp " SUBTRACT CPP /YX /Yc /Yu" \
        -perFileLine getThread_win32_$(ARCH).cpp " SUBTRACT CPP /YX /Yc /Yu" \
        -conditionalPerFileLine templateTable_$(ARCH).cpp " ADD CPP /Yu" "" \
        -additionalFile includeDB_adlc \
        -additionalFile includeDB_ci \
        -additionalFile includeDB_compiler1 \
        -additionalFile includeDB_compiler2 \
        -additionalFile includeDB_core \
        -additionalFile includeDB_gc \
        -additionalFile includeDB_gc_parallelScavenge \
        -additionalFile includeDB_gc_shared \
        -additionalFile includeDB_coreonly \
        -additionalGeneratedFile $(HOTSPOTBUILDSPACE) vm.def \
        -prelink Debug "Generating vm.def..." "cd $(HOTSPOTBUILDSPACE)\Debug	$(HOTSPOTMKSHOME)\sh $(HOTSPOTWORKSPACE)\build\windows\build_vm_def.sh	$(HOTSPOTMKSHOME)\mv vm.def .." \
        -prelink Release "Generating vm.def..." "cd $(HOTSPOTBUILDSPACE)\Release	$(HOTSPOTMKSHOME)\sh $(HOTSPOTWORKSPACE)\build\windows\build_vm_def.sh	$(HOTSPOTMKSHOME)\mv vm.def .." $(MakeDepsIncludesPRIVATE)

# Add in build-specific options
!if "$(ARCH)" == "i486"
MakeDepsIDEOptions=$(MakeDepsIDEOptions) -define IA32
!endif

!if "$(Variant)" == "compiler1"
MakeDepsIDEOptions=$(MakeDepsIDEOptions) -define COMPILER1 -perFileLine c1_RInfo_$(ARCH).cpp " SUBTRACT CPP /YX /Yc /Yu"
!endif

MakeDepsIDEOptions=$(MakeDepsIDEOptions)

#NOTE! This list must be kept in sync with GENERATED_NAMES in adlc.make.
!if "$(Variant)" == "compiler2"
MakeDepsIDEOptions=$(MakeDepsIDEOptions) -define COMPILER2 -additionalFile win32_$(ARCH).ad \
 -ignoreFile ad_$(ARCH).cpp \
 -additionalGeneratedFile $(HOTSPOTBUILDSPACE)/incls ad_$(ARCH).cpp \
 -ignoreFile ad_$(ARCH).hpp \
 -additionalGeneratedFile $(HOTSPOTBUILDSPACE)/incls ad_$(ARCH).hpp \
 -ignoreFile ad_$(ARCH)_clone.cpp \
 -additionalGeneratedFile $(HOTSPOTBUILDSPACE)/incls ad_$(ARCH)_clone.cpp \
 -ignoreFile ad_$(ARCH)_expand.cpp \
 -additionalGeneratedFile $(HOTSPOTBUILDSPACE)/incls ad_$(ARCH)_expand.cpp \
 -ignoreFile ad_$(ARCH)_format.cpp \
 -additionalGeneratedFile $(HOTSPOTBUILDSPACE)/incls ad_$(ARCH)_format.cpp \
 -ignoreFile ad_$(ARCH)_gen.cpp \
 -additionalGeneratedFile $(HOTSPOTBUILDSPACE)/incls ad_$(ARCH)_gen.cpp \
 -ignoreFile ad_$(ARCH)_misc.cpp \
 -additionalGeneratedFile $(HOTSPOTBUILDSPACE)/incls ad_$(ARCH)_misc.cpp \
 -ignoreFile ad_$(ARCH)_peephole.cpp \
 -additionalGeneratedFile $(HOTSPOTBUILDSPACE)/incls ad_$(ARCH)_peephole.cpp \
 -ignoreFile ad_$(ARCH)_pipeline.cpp \
 -additionalGeneratedFile $(HOTSPOTBUILDSPACE)/incls ad_$(ARCH)_pipeline.cpp \
 -ignoreFile adGlobals_$(ARCH).hpp \
 -additionalGeneratedFile $(HOTSPOTBUILDSPACE)/incls adGlobals_$(ARCH).hpp \
 -ignoreFile dfa_$(ARCH).cpp \
 -additionalGeneratedFile $(HOTSPOTBUILDSPACE)/incls dfa_$(ARCH).cpp \
 -perFileLine opcodes.cpp " SUBTRACT CPP /YX /Yc /Yu"
!endif

# Add in the jvmti (JSR-163) options
# NOTE: do not pull in jvmtiEnvRecommended.cpp.  This file is generated
#       so the programmer can diff it with jvmtiEnv.cpp to be sure the
#       code merge was done correctly (@see jvmti.make and jvmtiEnvFill.java).
#       If so, they would then check it in as a new version of jvmtiEnv.cpp.
MakeDepsIDEOptions=$(MakeDepsIDEOptions) \
 -absoluteInclude $(HOTSPOTBUILDSPACE)/jvmtifiles \
 -additionalGeneratedFile $(HOTSPOTBUILDSPACE)/jvmtifiles jvmdiEnter.cpp \
 -ignoreFile jvmdiEnter.cpp \
 -additionalGeneratedFile $(HOTSPOTBUILDSPACE)/jvmtifiles jvmdiEnterTrace.cpp \
 -ignoreFile jvmdiEnterTrace.cpp \
 -additionalGeneratedFile $(HOTSPOTBUILDSPACE)/jvmtifiles jvmtiEnv.hpp \
 -ignoreFile jvmtiEnv.hpp \
 -additionalGeneratedFile $(HOTSPOTBUILDSPACE)/jvmtifiles jvmtiEnter.cpp \
 -ignoreFile jvmtiEnter.cpp \
 -additionalGeneratedFile $(HOTSPOTBUILDSPACE)/jvmtifiles jvmtiEnterTrace.cpp \
 -ignoreFile jvmtiEnterTrace.cpp \
 -additionalGeneratedFile $(HOTSPOTBUILDSPACE)/jvmtifiles jvmti.h \
 -ignoreFile jvmti.h

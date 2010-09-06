# @(#)Makefile	1.24 03/12/23 16:35:36
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

WorkSpace=$(HOTSPOTWORKSPACE)

!ifdef ALT_BOOTDIR
BootStrapDir=$(ALT_BOOTDIR)
!else
!ifdef BOOTDIR
BootStrapDir=$(BOOTDIR)
!else
!ifdef JAVA_HOME
BootStrapDir=$(JAVA_HOME)
!endif
!endif
!endif

!include $(HOTSPOTWORKSPACE)/build/windows/makefiles/release.make
!include $(HOTSPOTWORKSPACE)/build/windows/makefiles/makedeps.make

# Pick up rules for building JVMTI/JVMDI (JSR-163)
JvmtiOutDir=jvmtifiles
!include $(HOTSPOTWORKSPACE)/build/windows/makefiles/jvmti.make

Platform=$(HOTSPOTWORKSPACE)/build/windows/platform_$(ARCH)

default:: includeDB.current vm.dsp Dependencies $(AdditionalTargets) $(JvmtiGeneratedFiles)

IncludeDBs=$(HOTSPOTWORKSPACE)/src/share/vm/includeDB_core
IncludeDBs=$(IncludeDBs) $(HOTSPOTWORKSPACE)/src/share/vm/includeDB_gc
IncludeDBs=$(IncludeDBs) $(HOTSPOTWORKSPACE)/src/share/vm/gc_implementation/includeDB_gc_parallelScavenge
IncludeDBs=$(IncludeDBs) $(HOTSPOTWORKSPACE)/src/share/vm/gc_implementation/includeDB_gc_shared

!if "$(Variant)" == "core"
IncludeDBs=$(IncludeDBs) \
           $(HOTSPOTWORKSPACE)/src/share/vm/includeDB_coreonly
!endif

!if "$(Variant)" == "compiler1"
IncludeDBs=$(IncludeDBs) \
           $(HOTSPOTWORKSPACE)/src/share/vm/includeDB_ci \
           $(HOTSPOTWORKSPACE)/src/share/vm/includeDB_compiler1
!endif

!if "$(Variant)" == "compiler2"
IncludeDBs=$(IncludeDBs) \
           $(HOTSPOTWORKSPACE)/src/share/vm/includeDB_ci \
           $(HOTSPOTWORKSPACE)/src/share/vm/includeDB_compiler2
# Pick up rules for building adlc
!include $(HOTSPOTWORKSPACE)/build/windows/makefiles/adlc.make
!endif

incls:
	@mkdir incls

includeDB.current vm.dsp Dependencies: local.make classes/MakeDeps.class $(IncludeDBs) incls
	@rm -f includeDB
	@cat $(IncludeDBs) > includeDB
	@$(RUN_JAVA) -Djava.class.path=classes MakeDeps diffs WinGammaPlatform $(Platform) includeDB.current $(Platform) includeDB $(MakeDepsOptions) $(MakeDepsIDEOptions)
	@rm -f includeDB.current
	@cp includeDB includeDB.current
	@touch vm.dsp

lists: classes/MakeDeps.class FORCE
	@if exist incls rmdir /s /q incls
	@rm -f includeDB
	@cat $(IncludeDBs) > includeDB
	@mkdir incls
	@$(RUN_JAVA) -Djava.class.path=classes MakeDeps WinGammaPlatform $(Platform) includeDB $(MakeDepsOptions) $(MakeDepsIDEOptions)
	@rm -f includeDB.current
	@cp includeDB includeDB.current
	@touch vm.dsp

clean:
	@rm -rf incls classes
	@rm -f includeDB includeDB.current vm.dsp Dependencies

classes/MakeDeps.class: $(MakeDepsSources)
	@if exist classes rmdir /s /q classes
	@mkdir classes
	@$(COMPILE_JAVAC) -classpath $(HOTSPOTWORKSPACE)\src\share\tools\MakeDeps -g -d classes $(MakeDepsSources)

FORCE:

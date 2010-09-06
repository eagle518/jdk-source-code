# @(#)Makefile	1.34 03/12/23 16:35:38
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

!include ../../local.make
!include $(WorkSpace)/build/windows/makefiles/release.make
!include $(WorkSpace)/build/windows/makefiles/makedeps.make
!include ../local.make
!include local.make

# Pick up rules for building JVMTI/JVMDI (JSR-163)
JvmtiOutDir=jvmtifiles
!include $(WorkSpace)/build/windows/makefiles/jvmti.make

# Pick up rules for building SA
!include $(WorkSpace)/build/windows/makefiles/sa.make

!if "$(Variant)" == "compiler2"
default:: includeDB.current Dependencies incls/ad_$(ARCH).cpp incls/dfa_$(ARCH).cpp $(JvmtiGeneratedFiles)
!else
default:: includeDB.current Dependencies $(JvmtiGeneratedFiles)
!endif

IncludeDBs=$(WorkSpace)/src/share/vm/includeDB_core $(WorkSpace)/src/share/vm/includeDB_gc $(WorkSpace)/src/share/vm/gc_implementation/includeDB_gc_parallelScavenge $(WorkSpace)/src/share/vm/gc_implementation/includeDB_gc_shared

!if "$(Variant)" == "core"
IncludeDBs=$(IncludeDBs) $(WorkSpace)/src/share/vm/includeDB_coreonly
!endif

!if "$(Variant)" == "compiler1"
IncludeDBs=$(IncludeDBs) $(WorkSpace)/src/share/vm/includeDB_ci
IncludeDBs=$(IncludeDBs) $(WorkSpace)/src/share/vm/includeDB_compiler1
!endif

!if "$(Variant)" == "compiler2"
IncludeDBs=$(IncludeDBs) $(WorkSpace)/src/share/vm/includeDB_ci
IncludeDBs=$(IncludeDBs) $(WorkSpace)/src/share/vm/includeDB_compiler2
!endif

# Note we don't generate a Visual C++ project file using MakeDeps for
# the batch build.
includeDB.current Dependencies: classes/MakeDeps.class $(IncludeDBs)
	cat $(IncludeDBs) > includeDB
	if exist incls rmdir /s /q incls
	mkdir incls
	$(RUN_JAVA) -Djava.class.path=classes MakeDeps WinGammaPlatform $(WorkSpace)/build/windows/platform_$(ARCH) includeDB $(MakeDepsOptions)
	rm -f includeDB.current
	cp includeDB includeDB.current

classes/MakeDeps.class: $(MakeDepsSources)
	if exist classes rmdir /s /q classes
	mkdir classes
	$(COMPILE_JAVAC) -classpath $(WorkSpace)\src\share\tools\MakeDeps -g -d classes $(MakeDepsSources)

!if "$(Variant)" == "compiler2"

!include $(WorkSpace)/build/windows/makefiles/adlc.make

!endif

!include $(WorkSpace)/build/windows/makefiles/shared.make

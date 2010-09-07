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

!include ../local.make
!include $(WorkSpace)/make/windows/makefiles/makedeps.make
!include local.make

# Pick up rules for building JVMTI (JSR-163)
JvmtiOutDir=jvmtifiles
!include $(WorkSpace)/make/windows/makefiles/jvmti.make

# Pick up rules for building SA
!include $(WorkSpace)/make/windows/makefiles/sa.make

!if ("$(Variant)" == "compiler2") || ("$(Variant)" == "tiered")
default:: includeDB.current Dependencies incls/ad_$(Platform_arch_model).cpp incls/dfa_$(Platform_arch_model).cpp $(JvmtiGeneratedFiles)
!else
default:: includeDB.current Dependencies $(JvmtiGeneratedFiles)
!endif

# core plus serial gc
IncludeDBs_base=$(WorkSpace)/src/share/vm/includeDB_core \
           $(WorkSpace)/src/share/vm/includeDB_jvmti \
           $(WorkSpace)/src/share/vm/includeDB_gc \
           $(WorkSpace)/src/share/vm/gc_implementation/includeDB_gc_serial

# parallel gc
IncludeDBs_gc= $(WorkSpace)/src/share/vm/includeDB_gc_parallel \
           $(WorkSpace)/src/share/vm/gc_implementation/includeDB_gc_parallelScavenge \
           $(WorkSpace)/src/share/vm/gc_implementation/includeDB_gc_shared \
           $(WorkSpace)/src/share/vm/gc_implementation/includeDB_gc_parNew \
           $(WorkSpace)/src/share/vm/gc_implementation/includeDB_gc_concurrentMarkSweep \
           $(WorkSpace)/src/share/vm/gc_implementation/includeDB_gc_g1

IncludeDBs_core=$(IncludeDBs_base) $(IncludeDBs_gc) \
                $(WorkSpace)/src/share/vm/includeDB_features

!if "$(Variant)" == "core"
IncludeDBs=$(IncludeDBs_core)
!endif

!if "$(Variant)" == "kernel"
IncludeDBs=$(IncludeDBs_base) $(WorkSpace)/src/share/vm/includeDB_compiler1
!endif

!if "$(Variant)" == "compiler1"
IncludeDBs=$(IncludeDBs_core) $(WorkSpace)/src/share/vm/includeDB_compiler1
!endif


!if "$(Variant)" == "compiler2"
IncludeDBs=$(IncludeDBs_core) $(WorkSpace)/src/share/vm/includeDB_compiler2
!endif

!if "$(Variant)" == "tiered"
IncludeDBs=$(IncludeDBs_core) $(WorkSpace)/src/share/vm/includeDB_compiler1 \
           $(WorkSpace)/src/share/vm/includeDB_compiler2
!endif

# Note we don't generate a Visual C++ project file using MakeDeps for
# the batch build.
includeDB.current Dependencies: classes/MakeDeps.class $(IncludeDBs)
	cat $(IncludeDBs) > includeDB
	if exist incls rmdir /s /q incls
	mkdir incls
	$(RUN_JAVA) -Djava.class.path=classes MakeDeps WinGammaPlatform$(VcVersion) $(WorkSpace)/make/windows/platform_$(BUILDARCH) includeDB $(MakeDepsOptions)
	rm -f includeDB.current
	cp includeDB includeDB.current

classes/MakeDeps.class: $(MakeDepsSources)
	if exist classes rmdir /s /q classes
	mkdir classes
	$(COMPILE_JAVAC) -classpath $(WorkSpace)\src\share\tools\MakeDeps -d classes $(MakeDepsSources)

!if ("$(Variant)" == "compiler2") || ("$(Variant)" == "tiered")

!include $(WorkSpace)/make/windows/makefiles/adlc.make

!endif

!include $(WorkSpace)/make/windows/makefiles/shared.make

#
# @(#)sa.make	1.12 04/04/28 14:31:28
#
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
#

# This makefile (sa.make) is included from the sa.make in the
# build directories.

# This makefile is used to build Serviceability Agent java code
# and generate JNI header file for native methods.

!include $(WorkSpace)/build/windows/makefiles/rules.make

AGENT_DIR = $(WorkSpace)/agent
!include $(WorkSpace)/build/sa.files

GENERATED = ..\generated

# tools.jar is needed by the JDI - SA binding
SA_CLASSPATH = $(BOOT_JAVA_HOME)\lib\tools.jar

# if WindbgHome is not defined or if BootStrapDir is not defined or 
# if $(AGENT_DIR) does not exist, we don't build SA.

default:: 
!if defined(WindbgHome) && defined(BootStrapDir)
	@if exist $(AGENT_DIR) $(MAKE) -f $(WorkSpace)\build\windows\makefiles\sa.make BootStrapDir=$(BootStrapDir) WorkSpace=$(WorkSpace) $(GENERATED)\sa-jdi.jar
!endif

$(GENERATED)\sa-jdi.jar: $(AGENT_ALLFILES:/=\) 
	@if not exist $(GENERATED)\saclasses mkdir $(GENERATED)\saclasses
	$(COMPILE_JAVAC) -source 1.4 -classpath $(SA_CLASSPATH) -g -d $(GENERATED)\saclasses $(AGENT_ALLFILES:/=\)
	$(COMPILE_RMIC) -classpath $(GENERATED)\saclasses -d $(GENERATED)\saclasses sun.jvm.hotspot.debugger.remote.RemoteDebuggerServer
	$(RUN_JAR) cvf $@ -C saclasses\ . 
	$(RUN_JAR) uvf $@ -C $(AGENT_SRC_DIR:/=\) META-INF\services\com.sun.jdi.connect.Connector 
	$(RUN_JAVAH) -classpath $(GENERATED)\saclasses -jni sun.jvm.hotspot.debugger.windbg.WindbgDebuggerLocal
	$(RUN_JAVAH) -classpath $(GENERATED)\saclasses -jni sun.jvm.hotspot.debugger.x86.X86ThreadContext 
	$(RUN_JAVAH) -classpath $(GENERATED)\saclasses -jni sun.jvm.hotspot.debugger.ia64.IA64ThreadContext 

cleanall :
	rm -rf $(GENERATED:\=/)/saclasses
	rm -rf $(GENERATED:\=/)/sa-jdi.jar
